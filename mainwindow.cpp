#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
	ui->setupUi(this);

	ui->leftTabWidget->setTabsClosable(false);
	ui->rightTabWidget->setTabsClosable(false);
	movementProgress = new ProgressDialog(this);

	readSettings();
	ui->commandsBox->setMain(this);

	connect(ui->commandsBox,SIGNAL(cdTo(QString)), this, SLOT(cdTo(QString)));
	connect(ui->commandsBox,SIGNAL(focusPreviouslyFocused()), this, SLOT(focusPreviouslyFocused()));
	connect(ui->quickBar,SIGNAL(cdTo(QString)), this, SLOT(cdTo(QString)));

	connect(ui->leftTabWidget,&CustomTabWidget::focusLost,[=](){
		leftFocusOut = true;
	});
	connect(ui->rightTabWidget,&CustomTabWidget::focusLost,[=](){
		leftFocusOut = false;
	});


	connect(ui->leftTabWidget,SIGNAL(gotResized(int,int,int)),ui->rightTabWidget,SLOT(sectionResized(int,int,int)));
	connect(ui->rightTabWidget,SIGNAL(gotResized(int,int,int)),ui->leftTabWidget,SLOT(sectionResized(int,int,int)));
	connect(ui->leftTabWidget,SIGNAL(gotMoved(int,int,int)),ui->rightTabWidget,SLOT(sectionMoved(int,int,int)));
	connect(ui->rightTabWidget,SIGNAL(gotMoved(int,int,int)),ui->leftTabWidget,SLOT(sectionMoved(int,int,int)));
	connect(ui->leftTabWidget, SIGNAL(currentChanged(int)), ui->leftTabWidget, SLOT(indexChanged(int)) );
	connect(ui->rightTabWidget, SIGNAL(currentChanged(int)), ui->rightTabWidget, SLOT(indexChanged(int)) );



	QSettings settings;
	auto headerState = settings.value("ProgressColumns").toByteArray();
	movementProgress->progress->tableWidget->horizontalHeader()->restoreState(headerState);

	ui->commandsBox->setEditable(true);
	//ui->commandsBox->addItem(" ");

	qDebug()<<QStandardPaths::AppConfigLocation;
}

MainWindow::~MainWindow(){
	writeSettings();
	delete ui;
}

void MainWindow::writeSettings(){
	QSettings settings;
	settings.beginGroup("MainWindow");
	settings.setValue("size", size());
	settings.setValue("pos", pos());
	settings.setValue("editor", editor);
	settings.endGroup();

	TabbedListView* current = (TabbedListView*)ui->leftTabWidget->currentWidget();
	settings.setValue("Columns", current->horizontalHeader()->saveState());
}

void MainWindow::readSettings(){
	QSettings settings;

	settings.beginGroup("MainWindow");
	resize(settings.value("size", QSize(400, 400)).toSize());
	move(settings.value("pos", QPoint(200, 200)).toPoint());
	editor = settings.value("editor", DEF_EDITOR).toString();
	settings.endGroup();

	ui->rightTabWidget->readSettings();
	ui->leftTabWidget->readSettings();
	ui->commandsBox->readSettings();
}

void MainWindow::keyPressEvent(QKeyEvent *event){
	auto key = event->key();
	switch (key) {
	case Qt::Key_F3:
		on_F3_clicked();
		break;
	case Qt::Key_F4:
		on_F4_clicked();
		break;
	case Qt::Key_F5:
		copyFiles();
		break;
	case Qt::Key_F6:
		moveFiles();
		break;
	case Qt::Key_F7:
		makeDir();
		break;
	case Qt::Key_F8:
		deleteFiles();
		break;
	default:
		QMainWindow::keyPressEvent(event);
		break;
	case Qt::Key_Tab:
		if(ui->leftTabWidget->currentWidget()->hasFocus())
			ui->rightTabWidget->setFocus();
		else
			ui->leftTabWidget->setFocus();

		break;
	}
}

void MainWindow::copyFiles(){

	QFileInfoList fileList = getSelectedFiles();
	if(fileList.isEmpty())
		return;
	QString destination = getDirInFocus(true);

	if(!getDir(destination, fileList.size(),COPY))
		return;

	QDir dir;
	if(!dir.exists(destination)){
		QString message = "Directory\n" + destination + "\ndoesn't exit.\nCreate it?";
		auto reply = QMessageBox::warning(this, "Warning!", message,
											QMessageBox::Yes|QMessageBox::No);
		if(reply == QMessageBox::No)
			return;
		dir.mkdir(destination);
	}

	movementProgress->show();
	movementProgress->setFileAction(fileList, destination, COPY);
}

void MainWindow::moveFiles(){
	QFileInfoList fileList = getSelectedFiles();
	if(fileList.isEmpty())
		return;
	QString destination = getDirInFocus(true);

	if(!getDir(destination, fileList.size(),MOVE))
		return;

	QDir dir;
	if(!dir.exists(destination)){
		QString message = "Directory\n" + destination + "\ndoesn't exit.\nCreate it?";
		auto reply = QMessageBox::warning(this, "Warning!", message,
											QMessageBox::Yes|QMessageBox::No);
		if(reply == QMessageBox::No)
			return;
		dir.mkdir(destination);
	}

	movementProgress->show();
	movementProgress->setFileAction(fileList, destination, MOVE);
}

void MainWindow::deleteFiles(){
	QFileInfoList fileList = getSelectedFiles();

	QString message = "Delete " + QString::number(fileList.size()) + " files?\n";

	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this, "Deleting ", message,
									QMessageBox::Yes|QMessageBox::No);

	if(reply == QMessageBox::No)
		return;

	bool status;
	foreach (auto fileInfo, fileList) {
		if(!fileInfo.fileName().compare("..", Qt::CaseInsensitive) )
			continue;

		if(!fileInfo.fileName().compare(".", Qt::CaseInsensitive) )
			continue;

		if(fileInfo.isDir()){
			QDir dir( fileInfo.absoluteFilePath() );
			status = dir.removeRecursively();
		}else
			status = QFile::remove( fileInfo.absoluteFilePath());

		if(!status){
			QString msg = "Unable to delede ";
			msg.append(fileInfo.filePath());
			QMessageBox::warning(this, "Error!",msg);
		}
	}
}

QString MainWindow::getDirInFocus(bool opposite){
	auto left = (TabbedListView*) ui->leftTabWidget->currentWidget();
	auto right = (TabbedListView*) ui->rightTabWidget->currentWidget();
	if(!(left->hasFocus() ^ opposite))
		return right->GetDirectory();
	else
		return left->GetDirectory();
}

QFileInfoList MainWindow::getSelectedFiles(){
	QFileInfoList list = getFocusedTab()->getSelectedFiles();
	if(1 == list.size() && 0==list.begin()->fileName().compare(".."))
		list.clear();
	return list;
}



TabbedListView* MainWindow::getFocusedTab(void){
	auto left = (TabbedListView*) ui->leftTabWidget->currentWidget();
	auto right = (TabbedListView*) ui->rightTabWidget->currentWidget();


	if(left->hasFocus())
		return left;
	return right;
}

void MainWindow::cdTo(const QString &dir){

	qDebug()<<"Got it!!!!";
	getFocusedTab()->cdTo(dir);
}

bool MainWindow::getDir(QString& dirName, int numFiles, ACTION action){

	QString message;

	switch (action) {
	case COPY:
		message = "<h3><font color=\"#0000ff\">Copy " + QString::number(numFiles) + " files to:</font><h3>";
		break;
	case MOVE:
		message = "<h3><font color=\"#ff0000\">Move " + QString::number(numFiles) + " files to:</font><h3>";
		break;
	default:
		message = "<h3><font color=\"#22b14c\">New directory name:</font><h3>";

	}

	QLabel lbl(this);
	NewDir *dialog = new NewDir(message, dirName,&lbl);
	dialog->adjustSize();
	lbl.show();

	QRect r = geometry();
	int x = r.x() + r.width()/2;
	int y = r.y() + r.height()/2;

	dialog->move(x,y);
	int hz = dialog->exec();
	if (hz) {
		dirName = dialog->dirName();
		lbl.setText(dirName);
		return true;
	}else
		return false;
}

void MainWindow::makeDir(){
	qDebug()<<getDirInFocus();
	QString dirName;
	if(!getDir(dirName))
		return;
	qDebug()<<dirName;
	QDir currDir(getDirInFocus());
	bool status = currDir.mkdir(dirName);
	if(!status)
		QMessageBox::critical(this,"Error!","Unable to create directory "+dirName+" in "+currDir.dirName());

}

void MainWindow::on_F3_clicked(){
	//QMessageBox::information(this,"Info","Not yet implemented");
	QFileInfoList fileList = getSelectedFiles();
	auto textViewer = new TextViewer(this);
	foreach (auto file, fileList)
		textViewer->setDocument(file.absoluteFilePath());
	textViewer->exec();

}

void MainWindow::on_F4_clicked(){

	QFileInfoList fileList = getSelectedFiles();
	foreach (auto file, fileList) {
		if(file.isDir())
			continue;
		QProcess *notepad = new QProcess();
		QStringList args;
		args<<file.absoluteFilePath();
		notepad->start(editor, args);
	}


}

void MainWindow::on_F5_clicked(){
	copyFiles();
}

void MainWindow::on_F6_clicked(){
	moveFiles();
}

void MainWindow::on_F8_clicked(){
	deleteFiles();
}

void MainWindow::on_F7_clicked(){
	makeDir();
}

void MainWindow::focusPreviouslyFocused(){
	if(leftFocusOut)
		ui->leftTabWidget->currentWidget()->setFocus();
	else
		ui->rightTabWidget->currentWidget()->setFocus();
}
