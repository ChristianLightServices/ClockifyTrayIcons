#ifndef CLOCKIFYUSER_H
#define CLOCKIFYUSER_H

#include <QObject>

#include <optional>

#include "TimeEntry.h"

class ClockifyUser : public QObject
{
	Q_OBJECT

public:
	explicit ClockifyUser(QString userId, QObject *parent = nullptr);
	explicit ClockifyUser(const ClockifyUser &that);

	bool hasRunningTimeEntry();
	TimeEntry getRunningTimeEntry();
	QDateTime stopCurrentTimeEntry(bool async = false) const;
	void startTimeEntry(const QString &projectId, bool async = false) const;
	void startTimeEntry(const QString &projectId, const QString &description, bool async = false) const;
	void startTimeEntry(const QString &projectId, QDateTime start, bool async = false) const;
	void startTimeEntry(const QString &projectId, const QString &description, QDateTime start, bool async = false) const;
	QVector<TimeEntry> getTimeEntries(std::optional<int> pageNumber = std::nullopt, std::optional<int> pageSize = std::nullopt);

	ClockifyUser &operator=(const ClockifyUser &other);

signals:

private:
	QString m_userId;
	QString m_name;
};

#endif // CLOCKIFYUSER_H
