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

/// Qt includes
#include <QColor>
#include <QDebug>
#include <QLinearGradient>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QtGlobal>
#include <QVariant>

/// CTK includes
#include "ctkTransferFunction.h"
#include "ctkTransferFunctionItems.h"

//-----------------------------------------------------------------------------
class ctkTransferFunctionItemPrivate : 
  public ctkPrivate<ctkTransferFunctionItem>
{
public:
  ctkTransferFunctionItemPrivate();

  QRectF               Rect;
  ctkTransferFunction* TransferFunction;
};

//-----------------------------------------------------------------------------
ctkTransferFunctionItemPrivate::ctkTransferFunctionItemPrivate()
{
  this->TransferFunction = 0;
}

//-----------------------------------------------------------------------------
ctkTransferFunctionItem::ctkTransferFunctionItem(QGraphicsItem* parentGraphicsItem)
  :QGraphicsObject(parentGraphicsItem)
{
  CTK_INIT_PRIVATE(ctkTransferFunctionItem);
}

//-----------------------------------------------------------------------------
ctkTransferFunctionItem::ctkTransferFunctionItem(
  ctkTransferFunction* transferFunction, QGraphicsItem* parentItem)
  :QGraphicsObject(parentItem)
{
  CTK_INIT_PRIVATE(ctkTransferFunctionItem);
  this->setTransferFunction(transferFunction);
}

//-----------------------------------------------------------------------------
ctkTransferFunctionItem::~ctkTransferFunctionItem()
{
  
}

//-----------------------------------------------------------------------------
void ctkTransferFunctionItem::setTransferFunction(ctkTransferFunction* transferFunction)
{
  CTK_D(ctkTransferFunctionItem);
  if (d->TransferFunction == transferFunction)
    {
    return;
    }
  d->TransferFunction = transferFunction;
  connect(d->TransferFunction, SIGNAL(changed()),
          this, SLOT(onTransferFunctionChanged()));
  this->update();
}

//-----------------------------------------------------------------------------
ctkTransferFunction* ctkTransferFunctionItem::transferFunction() const
{
  CTK_D(const ctkTransferFunctionItem);
  return d->TransferFunction;
}

//-----------------------------------------------------------------------------
void ctkTransferFunctionItem::onTransferFunctionChanged()
{
  this->update();
}

//-----------------------------------------------------------------------------
void ctkTransferFunctionItem::setRect(const QRectF& newRect)
{
  CTK_D(ctkTransferFunctionItem);
  if (d->Rect == newRect)
    {
    return;
    }
  d->Rect = newRect;
  this->update();
}

//-----------------------------------------------------------------------------
QRectF ctkTransferFunctionItem::rect() const
{
  CTK_D(const ctkTransferFunctionItem);
  return d->Rect;
}

//-----------------------------------------------------------------------------
QRectF ctkTransferFunctionItem::boundingRect()const
{
  CTK_D(const ctkTransferFunctionItem);
  return d->Rect;
}

//-----------------------------------------------------------------------------
QList<ctkPoint> ctkTransferFunctionItem::bezierParams(
  ctkControlPoint* start, ctkControlPoint* end) const
{
  Q_ASSERT(start);
  Q_ASSERT(end);
  QList<ctkPoint> points; 
  
  ctkBezierControlPoint* bezierCP = dynamic_cast<ctkBezierControlPoint*>(start);
  if (!bezierCP)
    {// just duplicate start and end into p1 and p2
    points << start->P;
    points << start->P;
    points << end->P;
    points << end->P;
    return points;
    }
  
  points << start->P;
  points << bezierCP->P1;
  points << bezierCP->P2;
  points << end->P;
  return points;
}

//-----------------------------------------------------------------------------
QList<ctkPoint> ctkTransferFunctionItem::nonLinearPoints(
  ctkControlPoint* start, ctkControlPoint* end) const
{
  Q_ASSERT(start);
    
  ctkNonLinearControlPoint* nonLinearCP = 
    dynamic_cast<ctkNonLinearControlPoint*>(start);
  if (!nonLinearCP)
    {
    QList<ctkPoint> points; 
    points << start->P;
    points << end->P;
    return points;
    }
  return nonLinearCP->SubPoints;
}

//-----------------------------------------------------------------------------
qreal ctkTransferFunctionItem::y(const QVariant& v) const
{ 
  Q_ASSERT(v.canConvert<qreal>() || v.canConvert<QColor>());
  if (v.canConvert<QColor>())
    {
    return v.value<QColor>().alphaF();
    }
  return v.toReal();
}

//-----------------------------------------------------------------------------
QColor ctkTransferFunctionItem::color(const QVariant& v) const
{ 
  Q_ASSERT(v.canConvert<QColor>());
  if (v.canConvert<QColor>())
    {
    return v.value<QColor>();
    }
  return QColor();
}

//-----------------------------------------------------------------------------
ctkTransferFunctionGradientItem::ctkTransferFunctionGradientItem(QGraphicsItem* parentGraphicsItem)
  :ctkTransferFunctionItem(parentGraphicsItem)
{
}

//-----------------------------------------------------------------------------
ctkTransferFunctionGradientItem::ctkTransferFunctionGradientItem(
  ctkTransferFunction* transferFunction, QGraphicsItem* parentItem)
  :ctkTransferFunctionItem(transferFunction, parentItem)
{
}

//-----------------------------------------------------------------------------
ctkTransferFunctionGradientItem::~ctkTransferFunctionGradientItem()
{  
}

//-----------------------------------------------------------------------------
void ctkTransferFunctionGradientItem::paint(
  QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
  int count = this->transferFunction() ? this->transferFunction()->count() : 0;
  if (count <= 0)
    {
    painter->fillRect(this->rect(),Qt::black);
    return;
    }
  qreal range[2];
  this->transferFunction()->range(range);
  qreal rangeDiff = this->rect().width() / (range[1] - range[0]);
  ctkControlPoint* startCP = this->transferFunction()->controlPoint(0);
  ctkControlPoint* endCP = 0;
  
  qreal start = startCP->x() * rangeDiff;
  qreal end = 0;
  for(int i = 1; i < count; ++i)
    {
    endCP = this->transferFunction()->controlPoint(i);
    // TODO, handle Bezier points for a finer gradient
    // TODO, handle nonlinear points
    if (dynamic_cast<ctkNonLinearControlPoint*>(startCP) != 0)
      {
      QList<ctkPoint> points = this->nonLinearPoints(startCP, endCP);
      for (int j = 1; j < points.count(); ++j)
        {
        end = points[j].X * rangeDiff;
        QLinearGradient gradient(start, 0, end, 0);
        gradient.setColorAt(0, this->color(points[j-1]));
        gradient.setColorAt(1, this->color(points[j]));
        QRectF itemRect = QRectF(start, 0, end - start, 
                                 this->rect().height());
        if (i==1 && j == 1)
          {
          itemRect.setLeft(0.);
          }
        if ((i == count -1) && (j == points.count() -1))
          {
          itemRect.setRight(this->rect().width());
          }
        painter->fillRect(itemRect, gradient);
        start = end;
        }
      }
    else
      {
      end = endCP->x() * rangeDiff;
      QLinearGradient gradient(start, 0, end, 0);
      gradient.setColorAt(0, this->color(startCP->value()));
      gradient.setColorAt(1, this->color(endCP->value()));
      QRectF itemRect = QRectF(start, 0, end - start, 
                               this->rect().height());
      if (i==1)
        {
        itemRect.setLeft(0.);
        }
      if (i == count -1)
        {
        itemRect.setRight(this->rect().width());
        }
      painter->fillRect(itemRect, gradient);
      }
    delete startCP;
    startCP = endCP;
    start = end;
    }
  if (startCP)
    {
    delete startCP;
    }
}

//-----------------------------------------------------------------------------
class ctkTransferFunctionControlPointsItemPrivate: 
  public ctkPrivate<ctkTransferFunctionControlPointsItem>
{
public:
  ctkTransferFunctionControlPointsItemPrivate();
  void init();
  QList<QPointF> ControlPoints;
  QSizeF         PointSize;
  int            SelectedPoint;
};

//-----------------------------------------------------------------------------
ctkTransferFunctionControlPointsItemPrivate::ctkTransferFunctionControlPointsItemPrivate()
{
  this->PointSize = QSizeF(10.,10.);
  this->SelectedPoint = -1;
}

//-----------------------------------------------------------------------------
void ctkTransferFunctionControlPointsItemPrivate::init()
{
  CTK_P(ctkTransferFunctionControlPointsItem);
  p->setAcceptedMouseButtons(Qt::LeftButton);
}

//-----------------------------------------------------------------------------
ctkTransferFunctionControlPointsItem::ctkTransferFunctionControlPointsItem(QGraphicsItem* parentGraphicsItem)
  :ctkTransferFunctionItem(parentGraphicsItem)
{
  CTK_INIT_PRIVATE(ctkTransferFunctionControlPointsItem);
  ctk_d()->init();
}

//-----------------------------------------------------------------------------
ctkTransferFunctionControlPointsItem::ctkTransferFunctionControlPointsItem(
  ctkTransferFunction* transferFunction, QGraphicsItem* parentItem)
  :ctkTransferFunctionItem(transferFunction, parentItem)
{
  CTK_INIT_PRIVATE(ctkTransferFunctionControlPointsItem);
  ctk_d()->init();
}

//-----------------------------------------------------------------------------
ctkTransferFunctionControlPointsItem::~ctkTransferFunctionControlPointsItem()
{  
}

//-----------------------------------------------------------------------------
void ctkTransferFunctionControlPointsItem::paint(
  QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
  CTK_D(ctkTransferFunctionControlPointsItem);
  int count = this->transferFunction() ? this->transferFunction()->count() : 0;
  if (count <= 0)
    {
    return;
    }
  qreal rangeX[2];
  this->transferFunction()->range(rangeX);
  qreal rangeXDiff = this->rect().width() / (rangeX[1] - rangeX[0]);
  QVariant rangeY[2];
  rangeY[0] = this->transferFunction()->minValue();
  rangeY[1] = this->transferFunction()->maxValue();
  qreal rangeYDiff = this->rect().height();
  if (rangeY[0].canConvert<qreal>())
    {
    rangeYDiff /= rangeY[1].toReal() - rangeY[0].toReal();
    }
  else if (rangeY[0].canConvert<qreal>())
    {
    rangeYDiff /= rangeY[1].value<QColor>().alphaF() - rangeY[0].value<QColor>().alphaF();
    }
  else
    {
    Q_ASSERT(rangeY[0].canConvert<qreal>() ||
             rangeY[0].canConvert<QColor>());
    }
  ctkControlPoint* startCP = this->transferFunction()->controlPoint(0);
  ctkControlPoint* endCP = 0;
  qreal start = 0;
  qreal end = 0;

  QPainterPath path;

  QPointF startPos(startCP->x(), this->y(startCP->value()));
  startPos.rx() *= rangeXDiff;
  startPos.setY(this->rect().height() 
                - startPos.y() * rangeYDiff);
  
  d->ControlPoints.clear();
  d->ControlPoints << startPos;

  path.moveTo(startPos);
  for(int i = 1; i < count; ++i)
    {
    endCP = this->transferFunction()->controlPoint(i);
    if (dynamic_cast<ctkNonLinearControlPoint*>(startCP))
      {
      QList<ctkPoint> points = this->nonLinearPoints(startCP, endCP);
      int j;
      for (j = 1; j < points.count(); ++j)
        {
        path.lineTo(
          QPointF(points[j].X * rangeXDiff, this->rect().height() - 
                  this->y(points[j].Value) * rangeYDiff));
        }
      j = points.count() -1;
      d->ControlPoints << QPointF(points[j].X * rangeXDiff, this->rect().height() - 
                  this->y(points[j].Value) * rangeYDiff);
      }
    else //dynamic_cast<ctkBezierControlPoint*>(startCP))
      {
      QList<ctkPoint> points = this->bezierParams(startCP, endCP);
      QList<ctkPoint>::iterator it = points.begin();
      QList<QPointF> bezierPoints;
      foreach(const ctkPoint& p, points)
        {
        bezierPoints << 
          QPointF(p.X * rangeXDiff, 
                  this->rect().height() - this->y(p.Value) * rangeYDiff);
        }
      path.cubicTo(bezierPoints[1], bezierPoints[2], bezierPoints[3]);
      d->ControlPoints << bezierPoints[3];
      }
    //qDebug() << i << points[0] << points[1] << points[2] << points[3];
    delete startCP;
    startCP = endCP;
    }
  if (startCP)
    {
    delete startCP;
    }
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setPen(QPen(QColor(255, 255, 255, 191), 1));
  painter->drawPath(path);

  QPainterPath points;
  foreach(const QPointF& point, d->ControlPoints)
    {
    points.addEllipse(point, d->PointSize.width(), d->PointSize.height());
    }
  painter->setBrush(QBrush(QColor(191, 191, 191, 127)));
  painter->drawPath(points);
}

//-----------------------------------------------------------------------------
void ctkTransferFunctionControlPointsItem::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
  CTK_D(ctkTransferFunctionControlPointsItem);
  QRectF pointArea(QPointF(0,0), d->PointSize*2.);
  d->SelectedPoint = -1;
  for(int i = 0; i < d->ControlPoints.count(); ++i)
    {
    pointArea.moveCenter(d->ControlPoints[i]);
    if (pointArea.contains(e->pos()))
      {
      d->SelectedPoint = i;
      break;
      }
    }
  if (d->SelectedPoint < 0)
    {
    e->ignore();
    }
}

//-----------------------------------------------------------------------------
void ctkTransferFunctionControlPointsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
  CTK_D(ctkTransferFunctionControlPointsItem);
  if (d->SelectedPoint < 0)
    {
    e->ignore();
    return;
    }
  qreal range[2];
  this->transferFunction()->range(range);
  qreal newPos = range[0] + e->pos().x() / (this->rect().width() / (range[1] - range[0]));
  newPos = qBound(range[0], newPos, range[1]);
  this->transferFunction()->setControlPointPos(d->SelectedPoint, newPos);
  //this->transferFunction()->setControlPointValue(d->SelectedPoint, e->y());
}

//-----------------------------------------------------------------------------
void ctkTransferFunctionControlPointsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
  CTK_D(ctkTransferFunctionControlPointsItem);
  if (d->SelectedPoint < 0)
    {
    e->ignore();
    return;
    }
  d->SelectedPoint = -1;
}
