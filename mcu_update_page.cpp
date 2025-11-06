/*
 * @Author       : kydin hikydin@gmail.com
 * @Date         : 2025-11-02 19:05:13
 * @LastEditors  : kydin hikydin@gmail.com
 * @LastEditTime : 2025-11-02 19:06:35
 * @FilePath     : \QPyocd\mcu_update_page.cpp
 * @Description  :
 * Copyright (c) 2025 by kydin, email: hikydin@gmail.com, All Rights Reserved.
 */
#include "mcu_update_page.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QButtonGroup>

static bool IsFileExist(QString fullFilePath)
{
	QFileInfo fileInfo(fullFilePath);
	if (fileInfo.exists())
	{
		return true;
	}
	return false;
}

static QString GetDateTime()
{
	QDateTime time = QDateTime::currentDateTime();
	QString time_str = time.toString("yyyy-MM-dd hh:mm:ss");
	return "[" + time_str + "] ";
}

McuUpdatePage::McuUpdatePage(QWidget *parent)
	: QWidget(parent), ui(new Ui::McuUpdatePageClass())
{
	ui->setupUi(this);
	this->setWindowTitle(tr("固件升级"));

	// 初始化接口对象
	interface_ = new McuUpdateInterface(this);

	// 初始化页面
	for (auto it = chip_pack_map.constBegin(); it != chip_pack_map.constEnd(); it++)
	{
		ui->cb_target_chip->addItem(it.key());
	}
	ui->cb_target_chip->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	ui->cb_probes->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	ui->te_log->setReadOnly(true);
	ui->le_firmware_path->setReadOnly(true);
	ui->progressBar->hide();
	ui->progressBar->setValue(0);
	QButtonGroup *bg_erase_chip = new QButtonGroup(this);
	bg_erase_chip->addButton(ui->rb_erase_chip, 0);
	bg_erase_chip->addButton(ui->rb_not_erase_chip, 1);
	bg_erase_chip->setExclusive(true);
	ui->rb_erase_chip->setChecked(true);

	// 如果文件不存在则禁用页面
	if (!CheckFile())
	{
		ui->btn_scan_probes->setDisabled(true);
		ui->btn_select_fiirmware->setDisabled(true);
		ui->btn_wirte_firmware->setDisabled(true);
	}

	// 连接信号与槽
	connect(ui->btn_scan_probes, &QPushButton::clicked, this, &McuUpdatePage::OnScanProbesClicked);
	connect(ui->btn_select_fiirmware, &QPushButton::clicked, this, &McuUpdatePage::OnSelectFiirmwareClicked);
	connect(ui->btn_wirte_firmware, &QPushButton::clicked, this, &McuUpdatePage::OnFirmwareUpdateClicked);
	connect(ui->cb_target_chip, &QComboBox::currentIndexChanged, this, &McuUpdatePage::CheckCanWriteFirmware);
	connect(ui->cb_probes, &QComboBox::currentIndexChanged, this, &McuUpdatePage::CheckCanWriteFirmware);
	connect(ui->le_firmware_path, &QLineEdit::textChanged, this, &McuUpdatePage::CheckCanWriteFirmware);
	connect(interface_, &McuUpdateInterface::UpdateProbes, this, &McuUpdatePage::UpdateProbes);
	connect(interface_, &McuUpdateInterface::DebugMessage, this, &McuUpdatePage::DebugMessage);
	connect(interface_, &McuUpdateInterface::ProgrammingUpdated, this, &McuUpdatePage::ProgrammingUpdated);
	connect(interface_, &McuUpdateInterface::FirmwareUpdateFinished, this, &McuUpdatePage::FirmwareUpdateFinished);

	// 页面初始化
	OnScanProbesClicked();
	CheckCanWriteFirmware();
}

McuUpdatePage::~McuUpdatePage()
{
	delete ui;
}

void McuUpdatePage::OnScanProbesClicked()
{
	LoadingPage();
	interface_->ScanProbe();
}

void McuUpdatePage::OnSelectFiirmwareClicked()
{
	QFileDialog *dialog = new QFileDialog(this);
	dialog->setWindowTitle(tr("选择固件文件"));
	dialog->setAcceptMode(QFileDialog::AcceptOpen);
	dialog->setFileMode(QFileDialog::ExistingFile);
	dialog->setViewMode(QFileDialog::Detail);

	QStringList filters;
	filters << "ELF Files (*.axf)";
	filters << "Hex Files (*.hex)";
	filters << "Binary Files (*.bin)";
	dialog->setNameFilters(filters);
	dialog->selectNameFilter(filters[0]);
	dialog->setDirectory(QDir::homePath());

	if (dialog->exec() == QDialog::Accepted)
	{
		QString file_path = dialog->selectedFiles()[0];
		if (!file_path.isEmpty() && IsFileExist(file_path))
		{
			QString filename = QFileInfo(file_path).fileName();
			ui->le_firmware_path->setText(file_path);
		}
	}
}

void McuUpdatePage::OnFirmwareUpdateClicked()
{
	FirmwareUpdateParam param;
	QString pack_path;
	QString target_chip = ui->cb_target_chip->currentText();
	QString firmware_path = ui->le_firmware_path->text();

	// 获取目标芯片包路径
	for (auto it = chip_pack_map.constBegin(); it != chip_pack_map.constEnd(); it++)
	{
		if (it.key() == target_chip)
		{
			QString path = QCoreApplication::applicationDirPath() + "/tools/packs/" + it.value();
			if (IsFileExist(path))
			{
				pack_path = path;
			}
			else
			{
				QMessageBox::critical(this, tr("错误"), tr("缺少对应芯片包"));
				return;
			}
		}
	}

	if (!IsFileExist(firmware_path))
	{
		QMessageBox::critical(this, tr("错误"), tr("固件文件不存在"));
		return;
	}

	param.pyocd_path = DEFAULT_PYOCD_PATH;
	param.is_erase_chip = ui->rb_erase_chip->isChecked() ? true : false;
	param.target_chip = target_chip;
	param.pack_path = pack_path;
	param.firmware_path = firmware_path;

	LoadingPage();
	ui->progressBar->setValue(0);
	ui->progressBar->show();
	interface_->StartFirmwareUpdate(param);
}

void McuUpdatePage::UpdateProbes(const QVector<ProbeInfo> &probes)
{
	UnLoadingPage();
	ui->cb_probes->clear();
	for (auto it = probes.begin(); it != probes.end(); it++)
	{
		ui->cb_probes->addItem(it->info);
	}
}

void McuUpdatePage::DebugMessage(const QString &msg)
{
	ui->te_log->append(GetDateTime() + msg);
}

void McuUpdatePage::ProgrammingUpdated(int percentage)
{
	ui->progressBar->setValue(percentage);
}

void McuUpdatePage::FirmwareUpdateFinished(bool success)
{
	if (success)
	{
		QMessageBox::information(this, tr("提示"), tr("固件升级完成"));
	}
	else
	{
		QMessageBox::critical(this, tr("错误"), tr("固件升级失败"));
	}
	UnLoadingPage();
	ui->progressBar->hide();
}

void McuUpdatePage::LoadingPage()
{
	ui->btn_select_fiirmware->setDisabled(true);
}

void McuUpdatePage::UnLoadingPage()
{
	ui->btn_select_fiirmware->setDisabled(false);
}

void McuUpdatePage::CheckCanWriteFirmware()
{
	bool is_valid = false;
	is_valid = (ui->cb_target_chip->count() > 0) ? true : false;
	is_valid = is_valid && (ui->cb_probes->count() > 0) ? true : false;
	is_valid = is_valid && !ui->le_firmware_path->text().isEmpty();
	ui->btn_wirte_firmware->setEnabled(is_valid);
}

bool McuUpdatePage::CheckFile()
{
	QString app_path = QCoreApplication::applicationDirPath();

	// check pyocd
	QString pyocd_path = DEFAULT_PYOCD_DIR_PATH + "pyocd.exe";
	if (!IsFileExist(DEFAULT_PYOCD_DIR_PATH))
	{
		QMessageBox::critical(this, tr("错误"), tr("缺少必要文件") + "pyocd.exe");
		return false;
	}

	QString pyocd_lib_path = DEFAULT_PYOCD_DIR_PATH + "_internal/";
	if (!IsFileExist(pyocd_lib_path))
	{
		QMessageBox::critical(this, tr("错误"), tr("缺少必要文件") + "_internal/");
		return false;
	}

	// check packs
	for (auto it = chip_pack_map.constBegin(); it != chip_pack_map.constEnd(); it++)
	{
		QString pack_path = app_path + "/tools/packs/" + it.value();
		if (!IsFileExist(pack_path))
		{
			QMessageBox::critical(this, tr("错误"), tr("缺少必要文件") + it.value());
			return false;
		}
	}
	return true;
}
