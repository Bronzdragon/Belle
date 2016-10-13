/* Copyright (C) 2012-2014 Carlos Pais
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "changecolor_editor_widget.h"

#include "changecolor.h"

#include <QHBoxLayout>

ChangeColorEditorWidget::ChangeColorEditorWidget(QWidget *parent) :
    ActionEditorWidget(parent)
{
    QHBoxLayout* layout = 0;

    mObjectsComboBox = new ObjectComboBox(this);

    QWidget* imageWidget = new QWidget(this);
    mImageCheckBox = new QCheckBox(imageWidget);
    mImageChooser = new ChooseFileButton(ChooseFileButton::ImageFilter, imageWidget);
    mImageChooser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    layout = new QHBoxLayout(imageWidget);
    layout->setContentsMargins(0, 2, 0, 1);
    layout->addWidget(mImageCheckBox, 0, Qt::AlignLeft);
    layout->addWidget(mImageChooser);

    QWidget* colorWidget = new QWidget(this);
    mColorCheckBox = new QCheckBox(colorWidget);
    mColorButton = new ColorPushButton(colorWidget);
    mColorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    layout = new QHBoxLayout(colorWidget);
    layout->setContentsMargins(0, 1, 0, 1);
    layout->addWidget(mColorCheckBox, 0, Qt::AlignLeft);
    layout->addWidget(mColorButton);

    QWidget* opacityWidget = new QWidget(this);
    mOpacityCheckBox = new QCheckBox(opacityWidget);
    mOpacitySlider = new QSlider(Qt::Horizontal, opacityWidget);
    mOpacitySlider->setMinimum(0);
    mOpacitySlider->setMaximum(255);

    layout = new QHBoxLayout(opacityWidget);
    layout->setContentsMargins(0, 1, 0, 2);
    layout->addWidget(mOpacityCheckBox, 0, Qt::AlignLeft);
    layout->addWidget(mOpacitySlider);

    beginGroup(tr("Change color editor"));
    appendRow(tr("Object"), mObjectsComboBox);
    appendRow(tr("Image"), imageWidget);
    appendRow(tr("Color"), colorWidget);
    appendRow(tr("Opacity"), opacityWidget);
    endGroup();
    resizeColumnToContents(0);

    connect(mObjectsComboBox, SIGNAL(objectChanged(Object*)), this, SLOT(onCurrentObjectChanged(Object*)));
    connect(mColorCheckBox, SIGNAL(toggled(bool)), this, SLOT(onColorCheckBoxToggled(bool)));
    connect(mColorButton, SIGNAL(colorChosen(const QColor&)), this, SLOT(onColorChosen(const QColor&)));
    connect(mOpacityCheckBox, SIGNAL(toggled(bool)), this, SLOT(onOpacityCheckBoxToggled(bool)));
    connect(mOpacitySlider, SIGNAL(valueChanged(int)), this, SLOT(onOpacityChanged(int)));
    connect(mImageCheckBox, SIGNAL(toggled(bool)), this, SLOT(onImageCheckBoxToggled(bool)));
    connect(mImageChooser, SIGNAL(fileSelected(const QString&)), this, SLOT(onFileSelected(const QString&)));

    mImageChooser->setVisible(false);
    mColorButton->setVisible(false);
    mOpacitySlider->setVisible(false);
}

void ChangeColorEditorWidget::updateData(GameObject* action)
{
    ActionEditorWidget::updateData(action);
    ChangeColor* changeColor = qobject_cast<ChangeColor*>(action);
    if (! changeColor)
        return;

    mObjectsComboBox->loadFromAction(changeColor);
    mImageCheckBox->setChecked(changeColor->isImageChangeEnabled());
    mImageChooser->setImageFile(changeColor->image());
    mColorCheckBox->setChecked(changeColor->isColorChangeEnabled());
    mColorButton->setColor(changeColor->color());
    mOpacityCheckBox->setChecked(changeColor->isOpacityChangeEnabled());
    mOpacitySlider->setValue(changeColor->opacity());
}

void ChangeColorEditorWidget::onColorChosen(const QColor & color)
{
    ChangeColor* changeColor = qobject_cast<ChangeColor*>(mGameObject);
    if (changeColor) {
        changeColor->setColor(color);
    }
}

void ChangeColorEditorWidget::onCurrentObjectChanged(Object* object)
{
    ChangeColor* changeColor = qobject_cast<ChangeColor*>(mGameObject);
    if (! changeColor)
        return;

    changeColor->setSceneObject(object);
}

void ChangeColorEditorWidget::onOpacityChanged(int value)
{
    ChangeColor* changeColor = qobject_cast<ChangeColor*>(mGameObject);
    if(changeColor)
        changeColor->setOpacity(value);
}

void ChangeColorEditorWidget::onFileSelected(const QString & filepath)
{
    ChangeColor* changeColor = qobject_cast<ChangeColor*>(mGameObject);
    if (changeColor)
        changeColor->setImage(filepath);
}

void ChangeColorEditorWidget::onImageCheckBoxToggled(bool checked)
{
    ChangeColor* changeColor = qobject_cast<ChangeColor*>(mGameObject);
    if (changeColor)
        changeColor->setImageChangeEnabled(checked);

    mImageChooser->setVisible(checked);
}

void ChangeColorEditorWidget::onColorCheckBoxToggled(bool checked)
{
    ChangeColor* changeColor = qobject_cast<ChangeColor*>(mGameObject);
    if (changeColor)
        changeColor->setColorChangeEnabled(checked);

    mColorButton->setVisible(checked);
}

void ChangeColorEditorWidget::onOpacityCheckBoxToggled(bool checked)
{
    ChangeColor* changeColor = qobject_cast<ChangeColor*>(mGameObject);
    if (changeColor)
        changeColor->setOpacityChangeEnabled(checked);

    mOpacitySlider->setVisible(checked);
}
