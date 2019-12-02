#ifndef PAINTWIDGET_H
#define PAINTWIDGET_H

#include <QWidget>
#include <QBrush>
#include <QFont>
#include <QPen>
#include <boost/optional.hpp>

#include <QtOpenGL>
#include <QPainter>
#include <vector>
class PaintWidget : public QGLWidget
{
    Q_OBJECT

public:
    PaintWidget(QWidget *parent);

private:
    qreal scale;
    QBrush background;
    QBrush circleBrush;
    QFont textFont;
    QPen circlePen;
    QPen textPen;
    QPointF cur_pos;
    QPointF center_;
    QPointF rect_start_point;
    QList<QLineF> segmentsInside_buffer;
    QList<QLineF> segm_buffer;
    QPainter painter;
    // QUERY
    boost::optional<QRectF> rect;
    boost::optional<QPointF> rect_end_point;
    boost::optional<QPointF> segm_start_point;
    boost::optional<QPointF> segm_end_point;
    boost::optional<QLineF> cur_segment;
    boost::optional<QPointF> move_point;

private:
    int checkRelRectCornerDiff(QPointF);
    bool rangeTest(qreal, qreal, qreal&, qreal&);
    bool clipLine2dLB(QLineF&);
    QPointF transform_to_paint_coords(QPointF& pt);
    QColor current_color_;
public slots:
    void animate();
   // void resizeGL(QResizeEvent*) override;
    void generateRandomSegments(int quantity = 10000);
    void reset();
protected:
    void wheelEvent ( QWheelEvent * event ) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *) override;
    void resizeGL(int w, int h) override;
    void initializeGL() override;
    void paintGL() override;
    QPointF screen_to_global(QPointF const & screen_pos) const;

};

#endif // PAINTWIDGET_H
