#include "newdir.h"

NewDir::NewDir(QString& dirName, QWidget *parent) :
	QDialog(parent)
{
	m_lineEdit = new QLineEdit(dirName, this);

	QPushButton *createButton = new QPushButton(tr("Ok"));
	createButton->setDefault(true);

	QPushButton *cancelButton = new QPushButton(tr("Cancel"));

	QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
	buttonBox->addButton(createButton, QDialogButtonBox::AcceptRole);
	buttonBox->addButton(cancelButton, QDialogButtonBox::RejectRole);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout *lt = new QVBoxLayout;
	lt->addWidget(m_lineEdit);
	lt->addWidget(buttonBox);

	setLayout(lt);
}

QString NewDir::dirName() const
{
	return m_lineEdit->text();
}
