#include "SelectDefaultProjectDialog.h"

#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

SelectDefaultProjectDialog::SelectDefaultProjectDialog(bool useLastProject,
													   bool useLastDescription,
													   QString oldDefaultProject,
													   QString oldDefaultDescription,
													   QPair<QStringList, QStringList> availableProjects,
													   QWidget *parent)
	: QDialog{parent},
	  m_projectButtons{new QButtonGroup{this}},
	  m_useLastProjectBtn{new QRadioButton{"Use the project from the last task", this}},
	  m_useSpecificProjectBtn{new QRadioButton{"Use a specific project every time", this}},
	  m_descriptionButtons{new QButtonGroup{this}},
	  m_useLastDescriptionBtn{new QRadioButton{"Use the description from the last task", this}},
	  m_useSpecificDescriptionBtn{new QRadioButton{"Use a specific description every time"}},
	  m_defaultProjectCombo{new QComboBox{this}},
	  m_defaultDescriptionEdit{new QLineEdit{oldDefaultDescription, this}},
	  m_useLastProject{useLastProject},
	  m_useLastDescription{useLastDescription},
	  m_defaultProject{(oldDefaultProject.isEmpty() ? availableProjects.first.first() : oldDefaultProject)},
	  m_availableProjects{availableProjects}
{
	auto buttons = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};

	auto layout	= new QVBoxLayout{this};

	auto projectGroup = new QGroupBox{"Default project settings", this};
	auto descriptionGroup = new QGroupBox{"Default description settings", this};

	auto projectGroupLayout = new QGridLayout{this};

	m_projectButtons->addButton(m_useLastProjectBtn);
	m_projectButtons->addButton(m_useSpecificProjectBtn);

	if (m_useLastProject)
	{
		m_useLastProjectBtn->setChecked(true);
		m_defaultProjectCombo->setEnabled(false);
	}
	else
		m_useSpecificProjectBtn->setChecked(true);

	m_defaultProjectCombo->addItems(m_availableProjects.second);
	m_defaultProjectCombo->setCurrentIndex(m_availableProjects.first.indexOf(m_defaultProject));

	projectGroupLayout->addWidget(m_useSpecificProjectBtn, 0, 0);
	projectGroupLayout->addWidget(m_defaultProjectCombo, 0, 1);
	projectGroupLayout->addWidget(m_useLastProjectBtn, 1, 0);

	projectGroup->setLayout(projectGroupLayout);

	auto descriptionGroupLayout = new QGridLayout{this};

	m_descriptionButtons->addButton(m_useLastDescriptionBtn);
	m_descriptionButtons->addButton(m_useSpecificDescriptionBtn);

	if (m_useLastDescription)
	{
		m_useLastDescriptionBtn->setChecked(true);
		m_defaultDescriptionEdit->setEnabled(false);
	}
	else
		m_useSpecificDescriptionBtn->setChecked(true);

	descriptionGroupLayout->addWidget(m_useSpecificDescriptionBtn, 0, 0);
	descriptionGroupLayout->addWidget(m_defaultDescriptionEdit, 0, 1);
	descriptionGroupLayout->addWidget(m_useLastDescriptionBtn, 1, 0);

	descriptionGroup->setLayout(descriptionGroupLayout);

	layout->addWidget(projectGroup);
	layout->addWidget(descriptionGroup);
	layout->addWidget(buttons);

	setLayout(layout);

	connect(m_projectButtons, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, [this](QAbstractButton *) {
		if (m_projectButtons->checkedButton() == static_cast<QAbstractButton *>(m_useSpecificProjectBtn))
		{
			m_useLastProject = false;
			m_defaultProjectCombo->setEnabled(true);
		}
		else
		{
			if (m_projectButtons->checkedButton() == static_cast<QAbstractButton *>(m_useLastProjectBtn))
				m_useLastProject = true;

			m_defaultProjectCombo->setEnabled(false);
		}
	});

	connect(m_defaultProjectCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
		m_defaultProject = m_availableProjects.first[m_defaultProjectCombo->currentIndex()];
	});

	connect(m_descriptionButtons, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, [this](QAbstractButton *) {
		if (m_descriptionButtons->checkedButton() == static_cast<QAbstractButton *>(m_useSpecificDescriptionBtn))
		{
			m_useLastDescription = false;
			m_defaultDescriptionEdit->setEnabled(true);
		}
		else
		{
			if (m_descriptionButtons->checkedButton() == static_cast<QAbstractButton *>(m_useLastDescriptionBtn))
				m_useLastDescription = true;

			m_defaultDescriptionEdit->setEnabled(false);
		}
	});

	connect(m_defaultDescriptionEdit, &QLineEdit::textChanged, this, [this]() {
		m_defaultDescription = m_defaultDescriptionEdit->text();
	});

	connect(buttons, &QDialogButtonBox::accepted, this, &SelectDefaultProjectDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &SelectDefaultProjectDialog::reject);
}
