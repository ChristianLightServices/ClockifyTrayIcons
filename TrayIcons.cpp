#include "TrayIcons.h"

#include <QTimer>
#include <QSettings>
#include <QMenu>
#include <QInputDialog>
#include <QDesktopServices>

#include <SingleApplication>

#include "ClockifyUser.h"
#include "JsonHelper.h"

const QByteArray WORKSPACE{"redacted"};
const QByteArray BREAKTIME{"redacted"};

TrayIcons::TrayIcons(const QSharedPointer<ClockifyManager> &manager, const QSharedPointer<ClockifyUser> &user, QObject *parent)
	: QObject{parent},
	  m_clockifyRunning{new QSystemTrayIcon},
	  m_runningJob{new QSystemTrayIcon},
	  m_manager{manager},
	  m_user{user}
{
	QSettings settings;

	setUpTrayIcons();

	m_eventLoop.setInterval(500);
	m_eventLoop.setSingleShot(false);
	m_eventLoop.callOnTimeout(this, &TrayIcons::updateTrayIcons);
	m_eventLoop.start();
}

TrayIcons::~TrayIcons()
{
	delete m_clockifyRunning;
	delete m_runningJob;
}

void TrayIcons::show()
{
	m_clockifyRunning->show();
	m_runningJob->show();
}

void TrayIcons::updateTrayIcons()
{
	QPair<QString, QIcon> clockifyOn{"Clockify is running", QIcon{":/greenpower.png"}};
	QPair<QString, QIcon> clockifyOff{"Clockify is not running", QIcon{":/redpower.png"}};
	QPair<QString, QIcon> onBreak{"You are on break", QIcon{":/yellowlight.png"}};
	QPair<QString, QIcon> working{"You are working", QIcon{":/greenlight.png"}};
	QPair<QString, QIcon> notWorking{"You are not working", QIcon{":/redlight.png"}};
	QPair<QString, QIcon> powerNotConnected{"You are offline", QIcon{":/graypower.png"}};
	QPair<QString, QIcon> runningNotConnected{"You are offline", QIcon{":/graylight.png"}};

	if (!m_manager->isConnectedToInternet())
	{
		m_clockifyRunning->setToolTip(powerNotConnected.first);
		m_runningJob->setToolTip(runningNotConnected.first);

		m_clockifyRunning->setIcon(powerNotConnected.second);
		m_runningJob->setIcon(runningNotConnected.second);
	}
	else if (m_user->hasRunningTimeEntry())
	{
		m_clockifyRunning->setToolTip(clockifyOn.first);
		m_clockifyRunning->setIcon(clockifyOn.second);

		if (m_user->getRunningTimeEntry()[0]["projectId"].get<QString>() == BREAKTIME)
		{
			m_runningJob->setToolTip(onBreak.first);
			m_runningJob->setIcon(onBreak.second);
		}
		else
		{
			m_runningJob->setToolTip(working.first);
			m_runningJob->setIcon(working.second);
		}
	}
	else
	{
		m_clockifyRunning->setToolTip(clockifyOff.first);
		m_runningJob->setToolTip(notWorking.first);

		m_clockifyRunning->setIcon(clockifyOff.second);
		m_runningJob->setIcon(notWorking.second);
	}
}

void TrayIcons::getNewProjectId()
{
	auto projects = m_manager->projects();
	QStringList projectIds;
	QStringList projectNames;
	for (const auto &project : projects)
	{
		projectIds.push_back(project.first);
		projectNames.push_back(project.second);
	}

	bool ok{true};
	do
	{
		auto projectName = QInputDialog::getItem(nullptr, "Default project", "Select your default project", projectNames, projectIds.indexOf(m_defaultProjectId), false, &ok);
		if (!ok)
			return;
		m_defaultProjectId = projectIds[projectNames.indexOf(projectName)];
	}
	while (m_defaultProjectId == "");

	QSettings settings;
	settings.setValue("projectId", m_defaultProjectId);
}

void TrayIcons::getNewApiKey()
{
	bool ok{true};
	do
	{
		m_apiKey = QInputDialog::getText(nullptr, "API key", "Enter your Clockify API key:", QLineEdit::Normal, m_apiKey, &ok);
		if (!ok)
			return;
	}
	while (m_apiKey == "");

	m_manager->setApiKey(m_apiKey);

	QSettings settings;
	settings.setValue("apiKey", m_apiKey);
}

void TrayIcons::setUpTrayIcons()
{
	auto m_clockifyRunningMenu = new QMenu;
	auto m_runningJobMenu = new QMenu;

	// set up the menu actions
	connect(m_clockifyRunningMenu->addAction("Start"), &QAction::triggered, this, [&]() {
		if (!m_user->hasRunningTimeEntry())
		{
			m_user->startTimeEntry(m_defaultProjectId);
			updateTrayIcons();
		}
	});
	connect(m_clockifyRunningMenu->addAction("Stop"), &QAction::triggered, this, [&]() {
		if (m_user->hasRunningTimeEntry())
		{
			m_user->stopCurrentTimeEntry();
			updateTrayIcons();
		}
	});
	connect(m_clockifyRunningMenu->addAction("Change default project"), &QAction::triggered, this, &TrayIcons::getNewProjectId);
	connect(m_clockifyRunningMenu->addAction("Change API key"), &QAction::triggered, this, &TrayIcons::getNewApiKey);
	connect(m_clockifyRunningMenu->addAction("Open the Clockify website"), &QAction::triggered, this, []() {
		QDesktopServices::openUrl(QUrl{"https://clockify.me/tracker/"});
	});
	connect(m_clockifyRunningMenu->addAction("Quit"), &QAction::triggered, qApp, &QApplication::quit);

	connect(m_runningJobMenu->addAction("Break"), &QAction::triggered, this, [&]() {
		QDateTime start = QDateTime::currentDateTimeUtc();

		if (m_user->hasRunningTimeEntry())
			start = m_user->stopCurrentTimeEntry();
		m_user->startTimeEntry(BREAKTIME, start);
		updateTrayIcons();
	});
	connect(m_runningJobMenu->addAction("Resume work"), &QAction::triggered, this, [&]() {
		QDateTime start = QDateTime::currentDateTimeUtc();

		if (m_user->hasRunningTimeEntry())
			start = m_user->stopCurrentTimeEntry();
		m_user->startTimeEntry(m_defaultProjectId, start);
		updateTrayIcons();
	});
	connect(m_runningJobMenu->addAction("Change default project"), &QAction::triggered, this, &TrayIcons::getNewProjectId);
	connect(m_runningJobMenu->addAction("Change API key"), &QAction::triggered, this, &TrayIcons::getNewApiKey);
	connect(m_runningJobMenu->addAction("Open the Clockify website"), &QAction::triggered, this, []() {
		QDesktopServices::openUrl(QUrl{"https://clockify.me/tracker/"});
	});
	connect(m_runningJobMenu->addAction("Quit"), &QAction::triggered, qApp, &QApplication::quit);

	// set up the actions on icon click
	connect(m_clockifyRunning, &QSystemTrayIcon::activated, this, [&](QSystemTrayIcon::ActivationReason reason) {
		if (reason != QSystemTrayIcon::Trigger && reason != QSystemTrayIcon::DoubleClick)
			return;

		if (m_user->hasRunningTimeEntry())
			m_user->stopCurrentTimeEntry();
		else
			m_user->startTimeEntry(m_defaultProjectId);
		updateTrayIcons();
	});
	connect(m_runningJob, &QSystemTrayIcon::activated, this, [&](QSystemTrayIcon::ActivationReason reason) {
		if (reason != QSystemTrayIcon::Trigger && reason != QSystemTrayIcon::DoubleClick)
			return;

		if (m_user->hasRunningTimeEntry())
		{
			if (m_user->getRunningTimeEntry()[0]["projectId"].get<QString>() == BREAKTIME)
			{
				auto time = m_user->stopCurrentTimeEntry();
				m_user->startTimeEntry(m_defaultProjectId, time);
			}
			else
			{
				auto time = m_user->stopCurrentTimeEntry();
				m_user->startTimeEntry(BREAKTIME, time);
			}
		}
		else
			m_user->startTimeEntry(m_defaultProjectId);
		updateTrayIcons();
	});

	m_clockifyRunning->setContextMenu(m_clockifyRunningMenu);
	m_runningJob->setContextMenu(m_runningJobMenu);

	updateTrayIcons();
}
