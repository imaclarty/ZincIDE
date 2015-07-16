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

#include "project.h"
#include "ui_mainwindow.h"
#include "courserasubmission.h"

#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QSortFilterProxyModel>

Project::Project(Ui::MainWindow *ui0) : ui(ui0), _courseraProject(NULL)
{
    projectFile = new QStandardItem("Untitled Project");
    invisibleRootItem()->appendRow(projectFile);
    /*
    mzn = new QStandardItem("Models");
    QFont font = mzn->font();
    font.setBold(true);
    mzn->setFont(font);
    invisibleRootItem()->appendRow(mzn);
    */
    zinc = new QStandardItem("Models");
    QFont font = zinc->font();
    font.setBold(true);
    zinc->setFont(font);
    invisibleRootItem()->appendRow(zinc);
    dzn = new QStandardItem("Data (right-click to run)");
    dzn->setFont(font);
    invisibleRootItem()->appendRow(dzn);
    other = new QStandardItem("Other");
    other->setFont(font);
    invisibleRootItem()->appendRow(other);
    _isModified = false;
}

Project::~Project() {
    delete _courseraProject;
}

void Project::setRoot(QTreeView* treeView, QSortFilterProxyModel* sort, const QString &fileName)
{
    if (fileName == projectRoot)
        return;
    _isModified = true;
    projectFile->setText(QFileInfo(fileName).fileName());
    projectFile->setIcon(QIcon(":/images/mznicon.png"));
    QStringList allFiles = files();
    //if (mzn->rowCount() > 0)
    //    mzn->removeRows(0,mzn->rowCount());
    if (zinc->rowCount() > 0)
        zinc->removeRows(0,zinc->rowCount());
    if (dzn->rowCount() > 0)
        dzn->removeRows(0,dzn->rowCount());
    if (other->rowCount() > 0)
        other->removeRows(0,other->rowCount());
    projectRoot = fileName;
    _files.clear();
    for (QStringList::iterator it = allFiles.begin(); it != allFiles.end(); ++it) {
        addFile(treeView, sort, *it);
    }
}

QVariant Project::data(const QModelIndex &index, int role) const
{
    if (role==Qt::UserRole) {
        QStandardItem* item = itemFromIndex(index);
        if (item->parent()==NULL || item->parent()==invisibleRootItem()) {
            if (item==projectFile) {
                return "00 - project";
            }
            //if (item==mzn) {
            //    return "01 - mzn";
            //}
            if (item==zinc) {
                return "01 - zinc";
            }
            if (item==dzn) {
                return "02 - dzn";
            }
            if (item==other) {
                return "03 - other";
            }
        }
        return QStandardItemModel::data(index,Qt::DisplayRole);
    } else {
        return QStandardItemModel::data(index,role);
    }
}

void Project::addFile(QTreeView* treeView, QSortFilterProxyModel* sort, const QString &fileName)
{
    if (_files.contains(fileName))
        return;
    QFileInfo fi(fileName);
    QString absFileName = fi.absoluteFilePath();
    QString relFileName;
    if (!projectRoot.isEmpty()) {
        QDir projectDir(QFileInfo(projectRoot).absoluteDir());
        relFileName = projectDir.relativeFilePath(absFileName);
    } else {
        relFileName = absFileName;
    }

    QStringList path = relFileName.split(QDir::separator());
    while (path.first().isEmpty()) {
        path.pop_front();
    }
    QStandardItem* curItem;
    bool isMiniZinc = true;
    bool isCoursera = false;
    //if (fi.suffix()=="mzn") {
    //    curItem = mzn;
    if (fi.suffix()=="zinc") {
        curItem = zinc;
    } else if (fi.suffix()=="dzn") {
        curItem = dzn;
    } else if (fi.suffix()=="fzn") {
        return;
    } else {
        curItem = other;
        isMiniZinc = false;
        isCoursera = fi.completeBaseName()=="_coursera";
    }

    if (isCoursera) {
        if (_courseraProject) {
            QMessageBox::warning(treeView,"Zinc IDE",
                                "Cannot add second Coursera options file",
                                QMessageBox::Ok);
            return;
        }
        QFile metadata(absFileName);
        if (!metadata.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(treeView,"Zinc IDE",
                                 "Cannot open Coursera options file",
                                 QMessageBox::Ok);
            return;
        }
        QTextStream in(&metadata);
        CourseraProject* cp = new CourseraProject;
        if (in.status() != QTextStream::Ok) {
            delete cp;
            goto coursera_done;
        }
        cp->course = in.readLine();
        if (in.status() != QTextStream::Ok) {
            delete cp;
            goto coursera_done;
        }
        cp->checkpwdSid= in.readLine();
        if (in.status() != QTextStream::Ok) {
            delete cp;
            goto coursera_done;
        }
        cp->name = in.readLine();
        QString nSolutions_s = in.readLine();
        int nSolutions = nSolutions_s.toInt();
        for (int i=0; i<nSolutions; i++) {
            if (in.status() != QTextStream::Ok) {
                delete cp;
                goto coursera_done;
            }
            QString line = in.readLine();
            QStringList tokens = line.split(", ");
            if (tokens.size() < 5) {
                delete cp;
                goto coursera_done;
            }
            CourseraItem item(tokens[0].trimmed(),tokens[1].trimmed(),tokens[2].trimmed(),
                              tokens[3].trimmed(),tokens[4].trimmed());
            cp->problems.append(item);
        }
        if (in.status() != QTextStream::Ok) {
            delete cp;
            goto coursera_done;
        }
        nSolutions_s = in.readLine();
        nSolutions = nSolutions_s.toInt();
        for (int i=0; i<nSolutions; i++) {
            if (in.status() != QTextStream::Ok) {
                delete cp;
                goto coursera_done;
            }
            QString line = in.readLine();
            QStringList tokens = line.split(", ");
            if (tokens.size() < 3) {
                delete cp;
                goto coursera_done;
            }
            CourseraItem item(tokens[0].trimmed(),tokens[1].trimmed(),tokens[2].trimmed());
            cp->models.append(item);
        }
        _courseraProject = cp;
        ui->actionSubmit_to_Coursera->setVisible(true);
    }
coursera_done:

    setModified(true, true);
    QStandardItem* prevItem = curItem;
    treeView->expand(sort->mapFromSource(curItem->index()));
    curItem = curItem->child(0);
    int i=0;
    while (curItem != NULL) {
        if (curItem->text() == path.first()) {
            path.pop_front();
            treeView->expand(sort->mapFromSource(curItem->index()));
            prevItem = curItem;
            curItem = curItem->child(0);
            i = 0;
        } else {
            i += 1;
            curItem = curItem->parent()->child(i);
        }
    }
    for (int i=0; i<path.size(); i++) {
        QStandardItem* newItem = new QStandardItem(path[i]);
        prevItem->appendRow(newItem);
        if (i<path.size()-1) {
            newItem->setIcon(QIcon(":/icons/images/folder.png"));
        } else {
            _files.insert(absFileName,newItem->index());
            if (isMiniZinc) {
                newItem->setIcon(QIcon(":/images/mznicon.png"));
            }
        }
        treeView->expand(sort->mapFromSource(newItem->index()));
        prevItem = newItem;
    }
}

QString Project::fileAtIndex(const QModelIndex &index)
{
    QStandardItem* item = itemFromIndex(index);
    if (item==NULL || item->hasChildren())
        return "";
    QString fileName;
    while (item != NULL && item->parent() != NULL && item->parent() != invisibleRootItem() ) {
        if (fileName.isEmpty())
            fileName = item->text();
        else
            fileName = item->text()+"/"+fileName;
        item = item->parent();
    }
    if (fileName.isEmpty())
        return "";
    if (!projectRoot.isEmpty()) {
        fileName = QFileInfo(projectRoot).absolutePath()+"/"+fileName;
    }
    QFileInfo fi(fileName);
    if (fi.canonicalFilePath().isEmpty())
        fileName = "/"+fileName;
    fileName = QFileInfo(fileName).canonicalFilePath();
    return fileName;
}

Qt::ItemFlags Project::flags(const QModelIndex& index) const
{
    if (index==editable) {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    } else {
        QStandardItem* item = itemFromIndex(index);
        if (!item->hasChildren() && (item==zinc || item==dzn || item==other) )
            return Qt::ItemIsSelectable;
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
}

QStringList Project::dataFiles(void) const
{
    QStringList ret;
    for (QMap<QString,QModelIndex>::const_iterator it = _files.begin(); it != _files.end(); ++it) {
        if (it.key().endsWith(".dzn"))
            ret << it.key();
    }
    return ret;
}

void Project::removeFile(const QString &fileName)
{
    if (fileName.isEmpty())
        return;
    if (!_files.contains(fileName)) {
        qDebug() << "Internal error: file " << fileName << " not in project";
        return;
    }
    setModified(true, true);
    QModelIndex index = _files[fileName];
    _files.remove(fileName);
    QStandardItem* cur = itemFromIndex(index);
    while (cur->parent() != NULL && cur->parent() != invisibleRootItem() && !cur->hasChildren()) {
        int row = cur->row();
        cur = cur->parent();
        cur->removeRow(row);
    }
    QFileInfo fi(fileName);
    if (fi.fileName()=="_coursera") {
        delete _courseraProject;
        _courseraProject = NULL;
        ui->actionSubmit_to_Coursera->setVisible(false);
    }
}

void Project::setEditable(const QModelIndex &index)
{
    editable = index;
}

void Project::setModified(bool flag, bool files)
{
    if (!projectRoot.isEmpty()) {
        if (_isModified != flag) {
            emit modificationChanged(flag);
            _isModified = flag;
            if (files) {
                _filesModified = _isModified;
            }
            if (!_isModified) {
                currentDataFileIndex(currentDataFileIndex(),true);
                haveZincArgs(haveZincArgs(),true);
                zincArgs(zincArgs(),true);
                n_solutions(n_solutions(),true);
                printAll(printAll(),true);
                printStats(printStats(),true);
                haveSolverFlags(haveSolverFlags(),true);
                solverFlags(solverFlags(),true);
                solverVerbose(solverVerbose(),true);
            }
        }
    }
}

bool Project::setData(const QModelIndex& index, const QVariant& value, int role)
{
    editable = QModelIndex();
    QString oldName = itemFromIndex(index)->text();
    if (oldName==value.toString())
        return false;
    QString filePath = QFileInfo(fileAtIndex(index)).canonicalPath();
    bool success = QFile::rename(filePath+"/"+oldName,filePath+"/"+value.toString());
    if (success) {
        _files[filePath+"/"+value.toString()] = _files[filePath+"/"+oldName];
        _files.remove(filePath+"/"+oldName);
        setModified(true, true);
        emit fileRenamed(filePath+"/"+oldName,filePath+"/"+value.toString());
        return QStandardItemModel::setData(index,value,role);
    } else {
        return false;
    }
}

bool Project::haveZincArgs(void) const
{
    return ui->conf_have_zinc_params->isChecked();
}
QString Project::zincArgs(void) const
{
    return ui->conf_zinc_params->text();
}
int Project::n_solutions(void) const
{
    return ui->conf_nsol->value();
}
bool Project::printAll(void) const
{
    return ui->conf_printall->isChecked();
}
bool Project::printStats(void) const
{
    return ui->conf_stats->isChecked();
}
bool Project::haveSolverFlags(void) const
{
    return ui->conf_have_solverFlags->isChecked();
}
QString Project::solverFlags(void) const
{
    return ui->conf_solverFlags->text();
}
bool Project::solverVerbose(void) const
{
    return ui->conf_solver_verbose->isChecked();
}

bool Project::isUndefined() const
{
    return projectRoot.isEmpty();
}

void Project::currentDataFileIndex(int i, bool init)
{
    if (init) {
        _currentDatafileIndex = i;
        ui->conf_data_file->setCurrentIndex(i);
    } else {
        checkModified();
    }
}

void Project::haveZincArgs(bool b, bool init)
{
    if (init) {
        _haveZincArgs = b;
        ui->conf_have_zinc_params->setChecked(b);
    } else {
        checkModified();
    }
}

void Project::zincArgs(const QString& a, bool init)
{
    if (init) {
        _zincArgs = a;
        ui->conf_zinc_params->setText(a);
    } else {
        checkModified();
    }
}

void Project::n_solutions(int n, bool init)
{
    if (init) {
        _n_solutions = n;
        ui->conf_nsol->setValue(n);
    } else {
        checkModified();
    }
}

void Project::printAll(bool b, bool init)
{
    if (init) {
        _printAll = b;
        ui->conf_printall->setChecked(b);
    } else {
        checkModified();
    }
}

void Project::printStats(bool b, bool init)
{
    if (init) {
        _printStats = b;
        ui->conf_stats->setChecked(b);
    } else {
        checkModified();
    }
}

void Project::haveSolverFlags(bool b, bool init)
{
    if (init) {
        _haveSolverFlags = b;
        ui->conf_have_solverFlags->setChecked(b);
    } else {
        checkModified();
    }
}

void Project::solverFlags(const QString& s, bool init)
{
    if (init) {
        _solverFlags = s;
        ui->conf_solverFlags->setText(s);
    } else {
        checkModified();
    }
}

void Project::solverVerbose(bool b, bool init)
{
    if (init) {
        _solverVerbose = b;
        ui->conf_solver_verbose->setChecked(b);
    } else {
        checkModified();
    }
}

int Project::currentDataFileIndex(void) const
{
    return ui->conf_data_file->currentIndex();
}

QString Project::currentDataFile(void) const
{
    return ui->conf_data_file->currentText();
}

void Project::checkModified()
{
    if (projectRoot.isEmpty() || _filesModified)
        return;
    if (currentDataFileIndex() != _currentDatafileIndex) {
        setModified(true);
        return;
    }
    if (haveZincArgs() != _haveZincArgs) {
        setModified(true);
        return;
    }
    if (zincArgs() != _zincArgs) {
        setModified(true);
        return;
    }
    if (n_solutions() != _n_solutions) {
        setModified(true);
        return;
    }
    if (printAll() != _printAll) {
        setModified(true);
        return;
    }
    if (printStats() != _printStats) {
        setModified(true);
        return;
    }
    if (haveSolverFlags() != _haveSolverFlags) {
        setModified(true);
        return;
    }
    if (solverFlags() != _solverFlags) {
        setModified(true);
        return;
    }
    if (solverVerbose() != _solverVerbose) {
        setModified(true);
        return;
    }
    setModified(false);
}

void Project::courseraError()
{
    QMessageBox::warning(NULL,"MiniZinc IDE",
                        "Error reading Coursera options file",
                        QMessageBox::Ok);

}
