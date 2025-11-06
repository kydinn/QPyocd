/*
 * @Author       : kydin hikydin@gmail.com
 * @Date         : 2025-11-02 19:05:13
 * @LastEditors  : kydin hikydin@gmail.com
 * @LastEditTime : 2025-11-02 19:13:42
 * @FilePath     : \QPyocd\mcu_update_page.h
 * @Description  : 
 * Copyright (c) 2025 by kydin, email: hikydin@gmail.com, All Rights Reserved.
 */
#pragma once

#include <QWidget>
#include "ui_mcu_update_page.h"

#include "mcu_update_interface.h"

static QMap<QString, QString> chip_pack_map = {
	{"CW32F030F8", "WHXY.CW32F030_DFP.1.0.4.pack"},
};

QT_BEGIN_NAMESPACE
namespace Ui
{
	class McuUpdatePageClass;
};
QT_END_NAMESPACE

class McuUpdatePage : public QWidget
{
	Q_OBJECT

public:
	McuUpdatePage(QWidget *parent = nullptr);
	~McuUpdatePage();

public slots:
	void OnScanProbesClicked();
	void OnSelectFiirmwareClicked();
	void OnFirmwareUpdateClicked();
	void UpdateProbes(const QVector<ProbeInfo> &probes);
	void DebugMessage(const QString &msg);
	void ProgrammingUpdated(int percentage);
	void FirmwareUpdateFinished(bool success);
	void LoadingPage();
	void UnLoadingPage();
	void CheckCanWriteFirmware();

private:
	bool CheckFile();

private:
	Ui::McuUpdatePageClass *ui;
	McuUpdateInterface *interface_;
};
