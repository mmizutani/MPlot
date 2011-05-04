#ifndef MPLOTTOOLS_CPP
#define MPLOTTOOLS_CPP

#include "MPlotTools.h"
#include "MPlotItem.h"
#include "MPlot.h"

#include <QDebug>


MPlotPlotSelectorTool::MPlotPlotSelectorTool() :
	MPlotAbstractTool()
{
	selectedItem_ = 0;
}

/// Returns the currently-selected item in the plot (0 if none).
MPlotItem* MPlotPlotSelectorTool::selectedItem() {
	return selectedItem_;
}


// This is used to detect PlotItem selection.
// If multiple items are on top of each other (or are within the selection range), this will alternate between them on successive clicks.

void MPlotPlotSelectorTool::mousePressEvent ( QGraphicsSceneMouseEvent * event ) {

	static unsigned int selIndex = 0;	// If two or more items are on top of each other, this is used to alternate between them
	MPlotItem* s;
	QList<MPlotItem*> selectedPossibilities;	// this will become a filtered list containing all the MPlotItem that are in range from this click.

	// Construct a rectangle "in the ballpark" of the mouse click:
	QRectF clickRegion(event->scenePos().x()-MPLOT_SELECTION_BALLPARK, event->scenePos().y()-MPLOT_SELECTION_BALLPARK, 2*MPLOT_SELECTION_BALLPARK, 2*MPLOT_SELECTION_BALLPARK);

	// Check all items for intersections
	foreach(MPlotItem* s2, plot()->plotItems() ) {

		// Have to verify that we actually intersect the shape... and that this guy is selectable
		if(s2->selectable() && s2->shape().intersects(s2->mapRectFromScene(clickRegion))) {

			selectedPossibilities << s2;	// add it to the list of selected possibilities
		}
	}

	// select from the list of possibilities using selIndex.  If there aren't any, s=0.
	if(selectedPossibilities.count() > 0)
		s = selectedPossibilities.at( (selIndex++) % selectedPossibilities.count() );
	else
		s = 0;

	// If we found one, and it's not the same as the old one:
	if(s && s != selectedItem_) {
		// tell the old item to unselect:
		if(selectedItem_)
			selectedItem_->setSelected(false);
		// Tell the new one to select:
		s->setSelected(true);
		// Assign, and emit signal:
		emit itemSelected(selectedItem_ = s);
	}

	// If the click didn't land on any item, and there was one previously selected:
	if(!s && selectedItem_) {
		// Tell the old one to unselect:
		selectedItem_->setSelected(false);
		selectedItem_ = 0;
		emit deselected();
	}

	// ignore the mouse press event, so that it will be propagated to other tools below us:
	event->ignore();

}

void MPlotPlotSelectorTool::mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseMoveEvent(event);
}

void MPlotPlotSelectorTool::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseReleaseEvent(event);
}

void MPlotPlotSelectorTool::wheelEvent ( QGraphicsSceneWheelEvent * event ) {
	QGraphicsObject::wheelEvent(event);
}

void MPlotPlotSelectorTool::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseDoubleClickEvent(event);
}




/// Constructor. By default, this tool operates on all axes (Left, Right, and Bottom), and adds/subtracts 25% to the axis range on each mousewheel click.  Use setZoomIncrement() and setYAxisTargets() to change these later.
MPlotWheelZoomerTool::MPlotWheelZoomerTool(qreal zoomIncrement)
	: MPlotAbstractTool()
{
	setZoomIncrement(zoomIncrement);
}

/// returns the fraction of the axis scale that will be added/subtracted on each mouse wheel click. (0.25 = 25% by default)
qreal MPlotWheelZoomerTool::zoomIncrement() const {
	return zf_;
}

/// set the zoom increment. On every mousewheel click, the range of the axis will be increased or decreased by this fraction.
void MPlotWheelZoomerTool::setZoomIncrement(qreal zi) {
	zf_ = fabs(zi);
}


void MPlotWheelZoomerTool::mousePressEvent ( QGraphicsSceneMouseEvent * event ) {
	event->ignore();
	QGraphicsObject::mousePressEvent(event);
}

/// Handles drag events, redraws the selection retangle to follow the mouse, and handles state transitions between dragStarted_ and dragInProgress_
void MPlotWheelZoomerTool::mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseMoveEvent(event);
}

void MPlotWheelZoomerTool::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseReleaseEvent(event);
}

/// The wheel zoomer implements scroll-in scroll-out zooming under the mouse cursor.
/*!
  <b>Basic equations: zooming in while maintaining a datapoint under the mouse cursor </b>

  -\c x: x datapoint value
  -\c min: previous axis minimum
  -\c max: previous axis maximum
  -\c min': new axis minimum
  -\c max': new axis maximum
  -\c F: zoom scale factor (ex: 0.6)

  1) The new scale is smaller than previous one; ie: multiplied by factor F

  \f[
	(max' - min') = F (max - min)
  \f]

  2) Also distance from \c x to \c min, as fraction of total axis range, stays constant

  \f[
	(x-min)/(max-min) = (x-min')/(max'-min')
  \f]

  Combining [1] and [2], and algebra:

  \f[
	min' = x + F(min - x)
  \f]
  \f[
	max' = x + F(max - x)
  \f]


   */
void MPlotWheelZoomerTool::wheelEvent ( QGraphicsSceneWheelEvent * event ) {


	// delta: mouse wheel rotation amount. 120 corresponds to 1 "click", or 15 degrees rotation on most mice.  Units are 1/8th of a degree.

	qreal F = (1 - qMin(zf_ * fabs(event->delta()) / 120, 0.9));

	// negative scrolling: we don't want a negative, we want the reciprocal: (zoom out, instead of in)
	if(event->delta() < 0)
		F = 1/F;

	foreach(MPlotAxisScale* axis, targetAxes_) {
		qreal drawingPos;
		if(axis->orientation() == Qt::Vertical) {
			drawingPos = event->pos().y();
		}
		else
			drawingPos = event->pos().x();

		qreal dataPos = axis->mapDrawingToData(drawingPos);

		qreal newMin, newMax;
		newMin = dataPos + F*(axis->min() - dataPos);
		newMax = dataPos + F*(axis->max() - dataPos);

		axis->setDataRange(MPlotAxisRange(newMin, newMax), false);
	}
}

void MPlotWheelZoomerTool::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseDoubleClickEvent(event);
}



/// Constructor.  \c axisTargets specifies an OR combination of MPlotAxis::AxisID flags that set which axes this tool has zoom control over.
MPlotDragZoomerTool::MPlotDragZoomerTool() :
	MPlotAbstractTool()
{

	selectionRect_ = new QGraphicsRectItem(QRectF(), this);

	QPen selectionPen = QPen(QBrush(MPLOT_SELECTION_COLOR), MPLOT_RUBBERBAND_WIDTH);
	selectionPen.setCosmetic(true);

	selectionRect_->setPen(selectionPen);

	QColor brushColor = MPLOT_SELECTION_COLOR;
	brushColor.setAlphaF(MPLOT_SELECTION_OPACITY);
	selectionRect_->setBrush(brushColor);

	dragInProgress_ = false;
	dragStarted_ = false;
}


void MPlotDragZoomerTool::mousePressEvent ( QGraphicsSceneMouseEvent * event ) {

	if(event->button() == Qt::LeftButton) {
		dragStarted_ = true;
		// don't display the rubberband rectangle until dragInProgress_
		// selectionRect_->setRect(QRectF(event->buttonDownPos(Qt::LeftButton), event->buttonDownPos(Qt::LeftButton)));
	}

}

/// Handles drag events, redraws the selection retangle to follow the mouse, and handles state transitions between dragStarted_ and dragInProgress_
void MPlotDragZoomerTool::mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) {

	// Possible transition: A drag event has started, and the user exceeded the drag deadzone to count as a real drag.
	if(dragStarted_) {
		QPointF dragDistance = event->buttonDownScenePos(Qt::LeftButton) - event->scenePos();

		// if we've gone far enough, this counts as a real drag:
		if(dragDistance.manhattanLength() > MPLOT_RUBBERBAND_DEADZONE) {
			// flag drag event in progress
			dragInProgress_ = true;
			dragStarted_ = false;

			// Disable auto-scaling on the plot... the user probably wants to take over manual control... (We don't want the plot jumping while they're auto-scaling it.)

			foreach(MPlotAxisScale* axis, targetAxes_) {
				axis->setAutoScaleEnabled(false);
			}
		}
	}

	// If we're dragging, draw/update the selection rectangle.
	// Figure out why this gets slow for high zoom settings.
	// attempted: Only do this if we've moved a true pixel, otherwise this gets really slow for high zoom settings.
	if(dragInProgress_ /*&& (event->buttonDownScenePos(Qt::LeftButton) - event->scenePos()).manhattanLength() > 1*/ ) {
		selectionRect_->setRect(QRectF(event->buttonDownPos(Qt::LeftButton), event->pos()));
	}
}

// Handles release events. If a drag was in progress and the user lets go of the left button, zoom to the new rectangle and save the old one on the recall stack.  If the user lets go of the right button, this is a restore to a zoom position on the stack.
void MPlotDragZoomerTool::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) {

	// left mouse button release: drag event is done.
	if(event->button() == Qt::LeftButton) {

		dragStarted_ = false;
		// disable the selection rectangle:
		selectionRect_->setRect(QRectF());

		if(dragInProgress_) {
			// This is a zoom change!
			dragInProgress_ = false;


			QList<QPair<MPlotAxisScale*, MPlotAxisRange> > oldZoomList;

			foreach(MPlotAxisScale* axis, targetAxes_) {
				oldZoomList << QPair<MPlotAxisScale*, MPlotAxisRange>(axis, axis->dataRange());

				double newRangeMin, newRangeMax;	// in drawing coordinates
				if(axis->orientation() == Qt::Vertical) {
					newRangeMin = event->buttonDownPos(Qt::LeftButton).y();
					newRangeMax = event->pos().y();
				}
				else {
					newRangeMin = event->buttonDownPos(Qt::LeftButton).x();
					newRangeMax = event->pos().x();
				}

				if(newRangeMin > newRangeMax)
					qSwap(newRangeMin, newRangeMax);

				axis->setDataRange(axis->mapDrawingToData(MPlotAxisRange(newRangeMin, newRangeMax)), false);
			}

			oldZooms_.push(oldZoomList);
		}
	}



	// Right mouse button: let's you go back to an old zoom setting
	if(!dragInProgress_ && event->button() == Qt::RightButton) {

		// do we have old zoom settings to jump back to?
		if(oldZooms_.count() > 0) {
			QList<QPair<MPlotAxisScale*, MPlotAxisRange> > oldZoomList = oldZooms_.pop();

			QPair<MPlotAxisScale*, MPlotAxisRange> oldZoomSetting;
			foreach(oldZoomSetting, oldZoomList) {
				MPlotAxisScale* axis = oldZoomSetting.first;
				if(targetAxes_.contains(axis))	// might have removed this axis scale as a target axis (active on this tool) since then. As a rule, we should only modify our target axes.
					axis->setDataRange(oldZoomSetting.second, false);
			}
		}

		// no old zoom settings. Go back to auto-scaling.
		else {
			foreach(MPlotAxisScale* axis, targetAxes_) {
				axis->setAutoScaleEnabled(true);
			}
		}
	}
}


void MPlotDragZoomerTool::wheelEvent ( QGraphicsSceneWheelEvent * event ) {
	QGraphicsObject::wheelEvent(event);
}

void MPlotDragZoomerTool::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseDoubleClickEvent(event);
}


MPlotCursorTool::MPlotCursorTool()
	: MPlotAbstractTool() {

}

MPlotCursorTool::~MPlotCursorTool() {
	foreach(MPlotPoint* c, cursors_) {
		if(plot())
			plot()->removeItem(c);
		delete c;
	}
	cursors_.clear();
}

unsigned MPlotCursorTool::numCursors() const {
	return cursors_.count();
}

QPointF MPlotCursorTool::value(unsigned cursorIndex) const {
	if(cursorIndex < numCursors())
		return cursors_.at(cursorIndex)->value();
	else
		return QPointF(0,0);
}

// Returns the MPlotPoint used to represent a specific cursor, so you can adjust it's color, marker, etc, or place it manually using MPlotPoint::setValue().
MPlotPoint* MPlotCursorTool::cursor(unsigned cursorIndex) const {
	if(cursorIndex < numCursors())
		return cursors_.at(cursorIndex);
	else
		return 0;
}

// remove a cursor.
void MPlotCursorTool::removeCursor() {
	if(numCursors() > 0) {
		MPlotPoint* removeMe = cursors_.takeLast();
		if(plot())
			plot()->removeItem(removeMe);
		delete removeMe;
	}
}

void MPlotCursorTool::addCursor(MPlotAxisScale* xAxisScale, MPlotAxisScale* yAxisScale, const QPointF& initialPos) {

	if(!plot()) {
		qWarning() << "MPlotCursorTool: You cannot add cursors to this tool until adding this tool to a plot.";
		return;
	}

	MPlotPoint* newCursor = new MPlotPoint();
	newCursor->setSelectable(false);

	if(!xAxisScale && yAxisScale)
		newCursor->setMarker(MPlotMarkerShape::HorizontalBeam, MPLOT_CURSOR_BIG_HACK);
	else if(xAxisScale && !yAxisScale)
		newCursor->setMarker(MPlotMarkerShape::VerticalBeam, MPLOT_CURSOR_BIG_HACK);
	else
		newCursor->setMarker(MPlotMarkerShape::Cross, MPLOT_CURSOR_BIG_HACK);


	plot()->addItem(newCursor);

	if(yAxisScale)
		newCursor->setYAxisTarget(yAxisScale);
	if(xAxisScale)
		newCursor->setXAxisTarget(xAxisScale);

	newCursor->setValue(initialPos);

	cursors_ << newCursor;
}



void MPlotCursorTool::mousePressEvent ( QGraphicsSceneMouseEvent * event ) {

	if(event->button() == Qt::LeftButton) {
		static unsigned activeCursor = 0;

		unsigned c = activeCursor % numCursors();

		MPlotPoint* cursor = cursors_.at(c);

		/// \todo clean this up... If a cursor was added prior to this tool being assigned to a plot, it won't be on the plot.  Add it here:
		if(cursor->plot() != plot())
			plot()->addItem(cursor);

		qreal y = event->pos().y();
		qreal x = event->pos().x();

		if(cursor->yAxisTarget())
			y = cursor->yAxisTarget()->mapDrawingToData(y);
		if(cursor->xAxisTarget())
			x = cursor->xAxisTarget()->mapDrawingToData(x);

		QPointF newPos(x, y);

		cursor->setValue(newPos);
		emit valueChanged(c, newPos);

		activeCursor++;
	}

	// ignore the mouse press event, so that it will be propagated to other tools below us:
	event->ignore();

}

void MPlotCursorTool::mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseMoveEvent(event);
}

void MPlotCursorTool::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseReleaseEvent(event);
}

void MPlotCursorTool::wheelEvent ( QGraphicsSceneWheelEvent * event ) {
	QGraphicsObject::wheelEvent(event);
}

void MPlotCursorTool::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseDoubleClickEvent(event);
}




#endif // MPLOTTOOLS_H

