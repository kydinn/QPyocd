/*
 * @Author       : kydin hikydin@gmail.com
 * @Date         : 2025-11-02 19:05:23
 * @LastEditors  : kydin hikydin@gmail.com
 * @LastEditTime : 2025-11-02 19:12:55
 * @FilePath     : \QPyocd\mcu_update_interface.cpp
 * @Description  : 
 * Copyright (c) 2025 by kydin, email: hikydin@gmail.com, All Rights Reserved.
 */
#include "mcu_update_interface.h"

#include "nlohmann/json.hpp"

#include <QRegularExpression>

using json = nlohmann::json;

McuUpdateInterface::McuUpdateInterface(QObject *parent)
    : QObject(parent)
{
    process_ = new QProcess(this);
    connect(process_, &QProcess::readyReadStandardOutput, this, &McuUpdateInterface::HandleStdOutput);
    connect(process_, &QProcess::readyReadStandardError, this, &McuUpdateInterface::HandleStdError);
    connect(process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &McuUpdateInterface::OnProcessFinished);
    connect(process_, &QProcess::errorOccurred, this, &McuUpdateInterface::OnProcessError);
}

McuUpdateInterface::~McuUpdateInterface()
{
}

void McuUpdateInterface::HandleStdOutput()
{
    switch (action_)
    {
    case Action_InProgress:
    {
        QByteArray data = process_->readAllStandardOutput();
        QString output = QString::fromUtf8(data);
        ParseFirmwareUpdateInfo(output); // 解析输出获取进度
        break;
    }
    default:
        break;
    }
}

void McuUpdateInterface::HandleStdError()
{
    QByteArray data = process_->readAllStandardError();
    QString error = QString::fromUtf8(data);
    emit DebugMessage("Error Output:" + error);
}

void McuUpdateInterface::OnProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    switch (action_)
    {
    case Action_ScanProbes:
    {
        probe_list_.clear();
        if (exitStatus == QProcess::NormalExit && exitCode == 0)
        {
            QByteArray output = process_->readAllStandardOutput();
            QString outputStr = QString::fromLocal8Bit(output);
            ParseProbeList(outputStr);
        }
        else
        {
            QByteArray errorOutput = process_->readAllStandardError();
            emit DebugMessage(QString::fromLocal8Bit(errorOutput));
        }
        emit DebugMessage("Scan probes finished");
        emit UpdateProbes(probe_list_);
        action_ = Action_Idle;
        break;
    }
    case Action_InProgress:
    {
        bool is_success = false;
        if (exitStatus == QProcess::NormalExit && exitCode == 0)
        {
            QByteArray output = process_->readAllStandardOutput();
            QString outputStr = QString::fromLocal8Bit(output);
            is_success = true;
            emit ProgrammingUpdated(100);
        }
        else
        {
            QByteArray errorOutput = process_->readAllStandardError();
            emit DebugMessage(QString::fromLocal8Bit(errorOutput));
            is_success = false;
        }
        percentage_ = 0;
        emit FirmwareUpdateFinished(is_success);
        emit DebugMessage("Firmware update finished");
        action_ = Action_Idle;
        break;
    }
    }
}

void McuUpdateInterface::OnProcessError(QProcess::ProcessError error)
{
    QString errorMsg;
    switch (error)
    {
    case QProcess::FailedToStart:
        errorMsg = "无法启动 pyocd 命令，请确保 pyocd 已安装并在 PATH 中";
        break;
    case QProcess::Crashed:
        errorMsg = "pyocd 进程意外退出";
        break;
    default:
        errorMsg = "执行 pyocd 命令时发生未知错误";
        break;
    }

    emit DebugMessage(errorMsg);
}

QVector<ProbeInfo> &McuUpdateInterface::GetAllProbe()
{
    return probe_list_;
}

void McuUpdateInterface::ScanProbe(QString pyocd_path)
{
    emit DebugMessage("Scanning probes...");
    action_ = Action_ScanProbes;
    process_->start(pyocd_path, QStringList() << "json");
}

void McuUpdateInterface::ParseProbeList(const QString &raw_output)
{
    QStringList probes;
    json j = json::parse(raw_output.toStdString(), nullptr, false);

    /*
    {
    "pyocd_version": "0.40.0",
    "version": {
        "major": 1,
        "minor": 1
    },
    "status": 0,
    "boards": [
        {
            "unique_id": "B8D4ECDC00E1",
            "info": "embedfire CMSIS-DAP",
            "board_vendor": null,
            "board_name": "Generic",
            "target": "cortex_m",
            "vendor_name": "embedfire",
            "product_name": "CMSIS-DAP"
        }
    ]
    }
    */
    QString pyocd_version = QString::fromStdString(j.value("pyocd_version", "unknown"));

    if (j.contains("boards") && j["boards"].is_array())
    {
        for (const auto &board : j["boards"])
        {
            ProbeInfo probe;
            probe.id = QString::fromStdString(board.value("unique_id", "unknown"));
            probe.info = QString::fromStdString(board.value("info", "unknown"));
            probe.target = QString::fromStdString(board.value("product_name", "unknown"));
            probe_list_.append(probe);
        }
    }
}

void McuUpdateInterface::StartFirmwareUpdate(FirmwareUpdateParam param)
{
    emit DebugMessage("Starting Firmware Update");
    QStringList arguments;
    arguments << "flash";
    if (param.is_erase_chip)
    {
        arguments << "--erase" << "chip";
    }
    if (!param.target_chip.isEmpty())
    {
        arguments << "--target" << param.target_chip;
    }
    if (!param.pack_path.isEmpty())
    {
        arguments << "--pack" << param.pack_path;
    }
    if (!param.firmware_path.isEmpty())
    {
        arguments << param.firmware_path;
    }
    else
    {
        emit DebugMessage("Firmware path is empty!");
        return;
    }

    qDebug() << "QProcess run command: [" << param.pyocd_path << " " << arguments << "]";
    action_ = Action_InProgress;
    process_->setProcessChannelMode(QProcess::MergedChannels); // 设置进程通道模式
    process_->start(param.pyocd_path, arguments);
}

void McuUpdateInterface::ParseFirmwareUpdateInfo(const QString &output)
{
    // pyOCD 通常输出类似 "[===    ] 45%" 的进度信息
    // 进度条通常有固定宽度，50 个字符
    if (output.contains("="))
    {
        percentage_ += output.count('=');
        int percentage = (percentage_ * 100) / 50;
        emit ProgrammingUpdated(percentage);
    }
}
