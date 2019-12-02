#include "paintWidget.h"

#include <QPainter>
#include <QTimer>
#include <QPaintEvent>
#pragma comment( lib, "OpenGL32.lib" )
#include <QTextStream>
#include <QRandomGenerator>

//! [0]
PaintWidget::PaintWidget(QWidget *parent)
    : QGLWidget(parent), scale(1.0)
{

    setMouseTracking(true);
 //   setFixedSize(200, 200);


    segmentsInside_buffer = QList<QLineF>();
    segm_buffer = QList<QLineF>();
    QLinearGradient gradient(QPointF(50, -20), QPointF(80, 20));
    gradient.setColorAt(0.0, Qt::white);
    gradient.setColorAt(1.0, QColor(164, 238, 223));

    background = QBrush(QColor(255, 255, 255));
    circleBrush = QBrush(gradient);
    circlePen = QPen(Qt::black);
    circlePen.setWidth(1);
    textPen = QPen(Qt::white);
    textFont.setPixelSize(50);
//glClearColor(255,255,255,1);
    move_point = boost::none;
    current_color_= QColor(0,0,0);
    QShortcut * shortcut = new QShortcut(QKeySequence(Qt::Key_Space),this,SLOT(reset()));
    shortcut->setAutoRepeat(false);
}


void PaintWidget::initializeGL()
{
    assert(doubleBuffer());
    setAutoBufferSwap(true);
    qglClearColor(Qt::white);

    //glEnable(GL_PROGRAM_POINT_SIZE);
}


void PaintWidget::animate()
{
    updateGL();
}


void PaintWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    if (rect)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1);
        glColor3f(0.0,0.0,1.0);
        glBegin(GL_LINE_LOOP);
            glVertex2d(rect->topLeft().x(), rect->topLeft().y());
            glVertex2d(rect->topRight().x(), rect->topRight().y());
            glVertex2d(rect->bottomRight().x(), rect->bottomRight().y());
            glVertex2d(rect->bottomLeft().x(), rect->bottomLeft().y());
        glEnd();

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    }
    if (segm_buffer.size()!=0)
    {
        glPointSize(6);
        glLineWidth(1);
        glColor3f(1.0,0.0,0.0);
        for (auto line: segm_buffer)
        {
            glBegin(GL_LINES);
            glVertex2d(line.x1(), line.y1());
            glVertex2d(line.x2(), line.y2());
            glEnd();
        }
        glColor3f(0.0,1.0,0.0);
        for (auto line: segmentsInside_buffer)
        {
            glBegin(GL_LINES);
            glVertex2d(line.x1(), line.y1());
            glVertex2d(line.x2(), line.y2());
            glEnd();
        }
        glColor3f(0.0,0.0,0.0);

    }
    if (cur_segment)
    {
        glBegin(GL_LINES);
        glVertex2d(cur_segment->x1(), cur_segment->y1());
        glVertex2d(cur_segment->x2(), cur_segment->y2());
        glEnd();
    }
}


void PaintWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button()==Qt::RightButton)
        move_point = boost::none;
    if (event->button()==Qt::LeftButton)
    {
        QPointF pos = event->pos();
        pos = screen_to_global(pos);
        if (!rect)
        {
            rect_start_point = pos;
            rect = QRectF(pos, pos);
        }
        else if (!rect_end_point)
            rect_end_point = pos;
        else if (!cur_segment)
            cur_segment = QLineF(pos, pos);
        else {
            cur_segment->setP2((pos));
            segm_buffer.push_back(cur_segment.value());
            QLineF lineToClip = cur_segment.value();
            if (clipLine2dLB(lineToClip))
                segmentsInside_buffer.push_back(lineToClip);
            cur_segment = boost::none;
        }

           updateGL();

    }

}




void PaintWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        move_point = screen_to_global(event->pos());
    else if (!rect)
        mouseReleaseEvent(event);

    updateGL();
    event->accept();
}


void PaintWidget::mouseMoveEvent(QMouseEvent* event)
{
  //  cur_pos = screen_to_global(event->pos());

    cur_pos = screen_to_global(event->pos());

    if (move_point)
    {
            const int w = size().width();
            const int h = size().height();

            QPointF pos(event->pos().x(), event->pos().y());
            QPointF sz(w / 2, h / 2);


            QPointF diff = pos - sz;
            diff.setX(-diff.x());

            center_ = *move_point + scale * diff;

            QTextStream cout(stdout);
            cout <<"event pos:" <<event->pos().x() << ' ' << event->pos().y() <<
                   "\nglobpos:" << pos.x() << ' ' << pos.y() <<
                   "\nmovepoint:"<<  move_point->x() << ' ' << move_point->y() <<
                    "\nsize:"<< size().width() << ' ' << size().height() <<

                   "\ncenterpoint:"<<  center_.x() << ' ' << center_.y() << "\n\n";



       //     move_point = pos;
            resizeGL(w, h);
             updateGL();

    }
    if (rect && !rect_end_point)
    {

        switch (checkRelRectCornerDiff(cur_pos))
        {
            case 0:         // down-right
                rect->setBottomRight(cur_pos);
                break;
            case 1:         // top-right
                rect->setTopRight(cur_pos);
                break;
            case 2:         // top-left
                rect->setTopLeft(cur_pos);
                break;
            case 3:         // bottom-left
                rect->setBottomLeft(cur_pos);
                break;
        }

         updateGL();
    }



    if (cur_segment)
    {
        cur_segment->setP2((cur_pos));
        updateGL();
    }


    event->accept();
}


void PaintWidget::wheelEvent ( QWheelEvent * e )
{
   // scale+=(e->delta()/(qreal)1200);

    double old_zoom = scale;

    int delta = e->delta() / 8 / 15;
    if (delta > 0)
    {
        for (int i = 0; i != delta; ++i)
            scale /= 1.05;
    }
    else if (delta < 0)
    {
        for (int i = 0; i != delta; --i)
            scale *= 1.05;
    }

    QPointF pos(e->pos().x(), e->pos().y());
    QPointF sz(size().width() / (qreal)2, size().height() / (qreal)2);

    QPointF diff = pos - sz;
    center_ += (old_zoom - scale) * QPointF(diff.x(), diff.y());
    mouseMoveEvent(new QMouseEvent(QEvent::MouseMove, screen_to_global(e->pos()),Qt::LeftButton, nullptr, nullptr));
    resizeGL(size().width(), size().height());



   updateGL();
   e->accept();
}

void PaintWidget::resizeGL(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    QPointF size = (scale / 2) * QPointF(w, h);

    QPointF left_bottom = center_ + (-size);
    QPointF right_top   = center_ + size;

    glOrtho(left_bottom.x(), right_top.x(), left_bottom.y(), right_top.y(), -1.0, 1.0);
    glViewport(0, 0, w, h);

}

QPointF PaintWidget::screen_to_global(QPointF const & screen_pos) const
{                                                   // global means (0,0) is at the center
    QPointF pos(screen_pos.x(), screen_pos.y());
    QPointF sz(size().width() / (qreal)2, size().height() / (qreal)2);

    QPointF diff = pos - sz;
    diff.setY(-diff.y());
    return center_+scale * diff;
}


int PaintWidget::checkRelRectCornerDiff(QPointF pt)
{
    qreal xs = rect_start_point.x();
    qreal ys = rect_start_point.y();
    qreal xe = pt.x();
    qreal ye = pt.y();
    if (xs < xe && ys < ye)
        return 0;
    else if (xs < xe && ys >= ye)
        return 1;
    else if (xs >= xe && ys >= ye)
        return 2;
    else return 3;
}

QPointF PaintWidget::transform_to_paint_coords(QPointF& pt)
{
    return QPointF(pt.x()/scale-center_.x(), pt.y()/scale-center_.y());
}



void PaintWidget::generateRandomSegments(int quantity)   {
    if (!rect_end_point)
        return;

    QRandomGenerator generatorAngle = QRandomGenerator::securelySeeded();
    QRandomGenerator generatorLength = QRandomGenerator::securelySeeded();
    QRandomGenerator generatorStartPoint = QRandomGenerator::securelySeeded();
    double minLength = 50;
    for (int i=0; i<quantity; ++i)
    {
        double angle = generatorAngle.bounded(360);
        double length = minLength + generatorLength.bounded(size().width());
        double start_x = generatorStartPoint.bounded(size().width());
        double start_y = generatorStartPoint.bounded(size().height());
        QPointF startPoint(start_x, start_y);
        startPoint = screen_to_global(startPoint);
        QLineF segment = QLineF(startPoint.x(), startPoint.y(), startPoint.x() + length*cos(angle),
                                    startPoint.y()+length*sin(angle));
        segm_buffer.push_back(segment);
        QLineF segm_clipped = QLineF(segment);
        if (clipLine2dLB(segm_clipped))
            segmentsInside_buffer.push_back(segm_clipped);

    }

    updateGL();

}




bool PaintWidget::rangeTest(qreal m, qreal c, qreal& tmin, qreal& tmax)
{
    qreal r;
    if (m < 0)
    {
        r = c / m;
        if (r > tmax)
            return false;
        if (tmin < r)
            tmin = r;
    }
    else if (m > 0)
    {
        r = c / m;
        if (r < tmin)
            return false;
        if (r < tmax)
            tmax = r;
    }
    else if (c < 0)
        return false;
    return true;


}

bool PaintWidget::clipLine2dLB(QLineF& segm)
{
    qreal tmin = 0;
    qreal tmax = 1;
    qreal dx = segm.dx();
    if (rangeTest(-dx, segm.x1() - rect->x(), tmin, tmax))
        if (rangeTest(dx, rect->topRight().x() - segm.x1(), tmin, tmax))
        {
            qreal dy = segm.dy();
            if (rangeTest(-dy, segm.y1() - rect->y(), tmin, tmax))
                if (rangeTest(dy, rect->bottomRight().y()-segm.y1(), tmin, tmax))
                {
                    if (tmax < 1)
                        segm.setP2(QPointF(segm.x1()+tmax*dx, segm.y1()+tmax*dy));
                    if (tmin > 0)
                        segm.setP1(QPointF(segm.x1()+tmin*dx, segm.y1()+tmin*dy));
                    return true;
                }
        }
    return false;

}


void PaintWidget::reset()
{

        segm_buffer.clear();
        segmentsInside_buffer.clear();
        rect = boost::none;
        rect_end_point = boost::none;
        cur_segment = boost::none;
       // center_ = QPointF(0,0);
       // scale = 1.;
        updateGL();
}
