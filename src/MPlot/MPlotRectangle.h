#ifndef MPLOTRECTANGLE_H
#define MPLOTRECTANGLE_H

#include "MPlotItem.h"
#include <QPen>
#include <QBrush>
class QPainter;

/// This type of MPlotItem plots a rectangle.  It's useful to highlight a rectangular region of interest on a plot.  The outline and fill color of the rectangle can be configured with setPen() and setBrush(), and the position and size of the rectangle (in data coordinates) controlled with setRect().
class MPlotRectangle : public MPlotItem
{
public:
	/// Default constructor.
	MPlotRectangle(const QRectF& rect, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual int rank() const { return 0; }

	/// Returns the pen used to draw the rectangle's outline
	QPen pen() const { return pen_; }
	/// Returns the brush used to fill in the rectangle.
	QBrush brush() const { return brush_; }
	/// Returns the position and size of the rectangle, in data coordinates. The  rectangle's QRectF::left() and QRectF::top() are be the minimum x and minimum y values, respectively.
	QRectF rect() const { return rect_; }

	/// Set the pen used to draw the rectangle's outline
	void setPen(const QPen& pen) {
		prepareGeometryChange();
		pen_ = pen;
		update();
	}

	/// Set the brush used to fill in the rectangle.  Try a semi-transparent brush for sexiness.
	void setBrush(const QBrush& brush) {
		brush_ = brush;
		update();
	}

	/// Set the coordinates of the rectangle that this item draws, in data coordinates.  The \c rectangle's QRectF::left() and QRectF::top() should be the minimum x and minimum y values, respectively.
	void setRect(const QRect& rectangle) {
		prepareGeometryChange();
		rect_ = rectangle;
		emitBoundsChanged();
		update();
	}


	/// Returns the extent of this rectangle, in data coordinates.
	virtual QRectF dataRect() const { return rect_; }
	/// Returns the extent of this rectangle, including selection and stroke borders, in drawing coordinates.
	virtual QRectF boundingRect() const;

	/// Required paint function
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);


	/// Re-implemented from MPlotItem to return the brush as the description color
	virtual QBrush legendColor() const { return brush_; }

protected:
	QPen pen_;
	QBrush brush_;
	QRectF rect_;
};

#endif // MPLOTRECTANGLE_H