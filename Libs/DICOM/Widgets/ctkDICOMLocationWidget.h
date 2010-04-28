/*=========================================================================

  Library:   CTK
 
  Copyright (c) 2010  Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 
=========================================================================*/

#ifndef __ctkDICOMLocationWidget_h
#define __ctkDICOMLocationWidget_h

// Qt includes 
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

#include "CTKDICOMWidgetsExport.h"

class ctkDICOMLocationWidgetPrivate;

class CTK_DICOM_WIDGETS_EXPORT ctkDICOMLocationWidget : public QWidget
{
public:
  typedef QWidget Superclass;
  explicit ctkDICOMLocationWidget(QWidget* parent=0);
  virtual ~ctkDICOMLocationWidget();

private:
  CTK_DECLARE_PRIVATE(ctkDICOMLocationWidget);
};

#endif
