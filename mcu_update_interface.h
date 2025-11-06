/*
 * @Author       : kydin hikydin@gmail.com
 * @Date         : 2025-11-02 19:05:23
 * @LastEditors  : kydin hikydin@gmail.com
 * @LastEditTime : 2025-11-02 19:12:58
 * @FilePath     : \QPyocd\mcu_update_interface.h
 * @Description  : 
 * Copyright (c) 2025 by kydin, email: hikydin@gmail.com, All Rights Reserved.
 */
#pragma once

#include <QObject>
#include <QProcess>
#include <QCoreApplication>

#define DEFAULT_PYOCD_DIR_PATH (QCoreApplication::applicationDirPath() + "/tools/pyocd/")
#define DEFAULT_PYOCD_PATH (DEFAULT_PYOCD_DIR_PATH + "pyocd.exe")

typedef struct _ProbeInfo {
	QString id;
	QString info;
	QString target;
} ProbeInfo;

typedef struct _FirmwareUpdateParam {
	QString pyocd_path;
	bool is_erase_chip;
	QString target_chip;
	QString pack_path;
	QString firmware_path;
} FirmwareUpdateParam;

typedef enum _ProcessActions {
	Action_Idle,
	Action_ScanProbes,
	Action_InProgress,
} ProcessActions;

class McuUpdateInterface  : public QObject
{
	Q_OBJECT

public:
	McuUpdateInterface(QObject *parent);
	~McuUpdateInterface();

	void HandleStdOutput();
	void HandleStdError();
	void OnProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void OnProcessError(QProcess::ProcessError error);
	QVector<ProbeInfo>& GetAllProbe();
	void ScanProbe(QString pyocd_path = DEFAULT_PYOCD_PATH);
	void ParseProbeList(const QString& raw_output);
	void StartFirmwareUpdate(FirmwareUpdateParam info);
	void ParseFirmwareUpdateInfo(const QString& output);

signals:
	void ProbesReady(const QStringList& probes);
	void ErrorOccurred(const QString& errorMessage);
	void ProgrammingUpdated(int percentage);
	void StatusUpdated(const QString& message);
	void FirmwareUpdateFinished(bool success);
	void UpdateProbes(const QVector<ProbeInfo>&);
	void DebugMessage(const QString& msg);

private:
	ProcessActions action_;
	QProcess* process_;
	QVector<ProbeInfo> probe_list_;
	int percentage_ = 0;
};

