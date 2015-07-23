/*
 *  Author:
 *     Guido Tack <guido.tack@monash.edu>
 *
 *  Copyright:
 *     NICTA 2013
 */

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PROJECT_H
#define PROJECT_H

#include <QSet>
#include <QStandardItemModel>
#include <QTreeView>

namespace Ui {
    class MainWindow;
}

class QSortFilterProxyModel;

class CourseraItem {
public:
    QString id;
    QString model;
    QString data;
    int timeout;
    QString name;
    CourseraItem(QString id0, QString model0, QString data0, QString timeout0, QString name0)
        : id(id0), model(model0), data(data0), timeout(timeout0.toInt()), name(name0) {}
    CourseraItem(QString id0, QString model0, QString name0)
        : id(id0), model(model0), timeout(-1), name(name0) {}
};

class CourseraProject {
public:
    QString name;
    QString checkpwdSid;
    QString course;
    QList<CourseraItem> problems;
    QList<CourseraItem> models;
};

class Project : public QStandardItemModel
{
    Q_OBJECT
public:
    Project(Ui::MainWindow *ui0);
    ~Project(void);
    void setRoot(QTreeView* treeView, QSortFilterProxyModel* sort, const QString& fileName);
    QVariant data(const QModelIndex &index, int role) const;
    void addFile(QTreeView* treeView, QSortFilterProxyModel* sort, const QString& fileName);
    void removeFile(const QString& fileName);
    QList<QString> files(void) const { return _files.keys(); }
    QString fileAtIndex(const QModelIndex& index);
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    QStringList dataFiles(void) const;
    void setEditable(const QModelIndex& index);
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );
    bool isProjectFile(const QModelIndex& index) { return projectFile->index()==index; }
    bool isModified() const { return _isModified; }
    void setModified(bool flag, bool files=false);

    int currentDataFileIndex(void) const;
    QString currentDataFile(void) const;
    int currentDataFile2Index(void) const;
    QString currentDataFile2(void) const;
    bool haveZincArgs(void) const;
    QString zincArgs(void) const;
    int n_solutions(void) const;
    bool printAll(void) const;
    bool printStats(void) const;
    bool haveSolverFlags(void) const;
    QString solverFlags(void) const;
    bool solverVerbose(void) const;
    CourseraProject& coursera(void) { return *_courseraProject; }
    bool isUndefined(void) const;
public slots:
    void currentDataFileIndex(int i, bool init=false);
    void currentDataFile2Index(int i, bool init=false);
    void haveZincArgs(bool b, bool init=false);
    void zincArgs(const QString& a, bool init=false);
    void n_solutions(int n, bool init=false);
    void printAll(bool b, bool init=false);
    void printStats(bool b, bool init=false);
    void haveSolverFlags(bool b, bool init=false);
    void solverFlags(const QString& s, bool init=false);
    void solverVerbose(bool b, bool init=false);
signals:
    void fileRenamed(const QString& oldName, const QString& newName);
    void modificationChanged(bool);
protected:
    Ui::MainWindow *ui;
    bool _isModified;
    bool _filesModified;
    QString projectRoot;
    QMap<QString, QModelIndex> _files;
    QStandardItem* projectFile;
    //QStandardItem* mzn;
    QStandardItem* zinc;
    QStandardItem* dzn;
    QStandardItem* other;
    QModelIndex editable;

    int _currentDatafileIndex;
    int _currentDatafile2Index;
    bool _haveZincArgs;
    QString _zincArgs;
    int _n_solutions;
    bool _printAll;
    bool _printStats;
    bool _haveSolverFlags;
    QString _solverFlags;
    bool _solverVerbose;
    CourseraProject* _courseraProject;

    void checkModified(void);
    void courseraError(void);
};

#endif // PROJECT_H
