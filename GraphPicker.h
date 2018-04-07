/**
   \file


   \author Javier Gonzalez
   \version $Id: TEMPLATE.h.tpl,v 1.5 2003/09/25 14:38:19 lukas Exp $
   \date 29 Aug 2011

   TODO:
    - logarithmic scale
    - errors (both symmetric and asymetric)
    - Clear
    - undo
*/

#ifndef __imageviewer_h_
#define __imageviewer_h_

static const char CVSId__imageviewer[] =
"$Id$";

#include <QMainWindow>
#include <QScrollArea>
#include <vector>
#include <QPointF>
#include <QtGui>
#include <QGenericMatrix>
#include <ostream>

class QAction;
class QLabel;
class QMenu;
class QScrollBar;
class QMouseEvent;

namespace picker {

  typedef QGenericMatrix<2,2, qreal> Matrix;

  class Vector: public QGenericMatrix<1,2, qreal> {
  public:
    Vector(const QGenericMatrix<1,2, qreal>& values)
    {
      this->data()[0] = values.data()[0];
      this->data()[1] = values.data()[1];
    }
    Vector(const qreal* values):
      QGenericMatrix<1,2, qreal>(values)
    {}
    Vector()
    {
      this->data()[0] = 0.;
      this->data()[1] = 0.;
    }
    Vector(qreal sx, qreal sy)
    {
      this->data()[0] = sx;
      this->data()[1] = sy;
    }
    Vector(QPointF p)
    {
      this->data()[0] = p.x();
      this->data()[1] = p.y();
    }

    Vector& operator=(const Vector& other)
    {
      this->data()[0] = other.data()[0];
      this->data()[1] = other.data()[1];
      return *this;
    }

    qreal x() const {return (*this)(0,0);}
    qreal y() const {return (*this)(1,0);}
  };

  class Point {
  public:
    // Point(QPointF p):
    //   pos(p)
    // {}

    Point(Vector p):
      pos(p)
    {}

    Vector pos;
    Vector dxPlus;
    Vector dxMinus;
    Vector dyPlus;
    Vector dyMinus;
  };

  class TopLayer;

  class PointCollection {
  public:
    PointCollection(TopLayer* parent);
    ~PointCollection(){}

    void AddPoint(double x, double y)
    {
      AddPoint(Vector(x,y));
    }
    void AddPoint(Vector pos);

    Point& GetPoint(unsigned int i)
    {return fPoints[i];}
    const Point& GetPoint(unsigned int i) const
    {return fPoints[i];}

    void Paint(QPainter& painter);

    template<class PointType>
    bool Intersects(PointType pos) const;

    template<class PointType>
    Point& GetPoint(PointType pos);
    template<class PointType>
    const Point& GetPoint(PointType pos) const;

    unsigned int GetSize() const
    { return fPoints.size(); }

    void Clear()
    { fPoints.clear(); }

  private:
    template<class PointType>
    int IntersectingIndex(PointType pos) const;

    std::vector<Point> fPoints;

    int fMarkerSize;
    QBrush fBrush;
    QPen fPen;

    TopLayer* fParent;
  };

  class TopLayer: public QWidget {

    Q_OBJECT
  public:
    TopLayer(QWidget * parent = 0);
    ~TopLayer(){}

    const PointCollection& GetPoints() const
    { return fPoints; }

    Vector FromPixelToTarget(const Vector& point) const;
    Vector FromTargetToPixel(const Vector& point) const;

    const Matrix& SizeMatrix() const;
    const Matrix& InvSizeMatrix() const;

    const Matrix& PixelToTargetMatrix()
    { return fPixelToTarget; }
    const Matrix& TargetToPixelMatrix()
    { return fTargetToPixel; }

  public slots:
    void SetLogX(bool v)
    { fLogX=v; Calibrate(); }
    void SetLogY(bool v)
    { fLogY=v; Calibrate(); }
    //void SetCalibrated(bool c);
    void Reset();
    void Clear();
    void ShowCrosshairs(bool show);
    void ResetCrosshairs();

  protected:
    virtual void keyReleaseEvent(QKeyEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void paintEvent(QPaintEvent* event);

  private:
    void Calibrate();

    bool fIsCalibrated;
  public:
    bool fLogX;
    bool fLogY;
    bool fShowCrosshairs;

  private:
    QPointF fMousePos;
    Vector fXaxis;
    Vector fYaxis;

    bool fCustomXaxis;
    bool fCustomYaxis;

    bool fSettingAxis;
    bool fMousePressed;
    Vector fEnd;
    Vector fStart;

    PointCollection fPoints;

    std::vector<Vector> fTargetBase;
    std::vector<Vector> fPixelBase;

    std::vector<Vector> fCalibrationPointsImage;
    std::vector<Vector> fCalibrationPointsTarget;

    Matrix fPixelToTarget;
    Matrix fTargetToPixel;

    Matrix fCalibToPixel;
    Matrix fPixelToCalib;

  };


  class GraphPicker : public QMainWindow
  {
    Q_OBJECT

  public:
    GraphPicker(std::string filename = "");

  private slots:
    void open(std::string filename = "");
    void save();
    void about();
    //void ToggleCalibrate();
    void Clear();
    void SetLogX();
    void SetLogY();

  private:
    void createActions();
    void createMenus();
    void createToolbar();
    void updateActions();
    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    QLabel* fImageLabel;
    TopLayer* fTopWidget;

    QScrollArea* fScrollArea;

    QAction *openAct;
    QAction *saveAct;
    QAction *exitAct;
    QAction *aboutAct;
    QAction *aboutQtAct;

    QAction *clearAct;
    QAction *calibrateAct;
    QAction *logYAct;
    QAction *logXAct;
    QAction *crossAct;
    QAction *resetCrossAct;

    QToolBar* fToolbar;
    QMenu *fFileMenu;
    QMenu *fHelpMenu;
  };

  template<int M, int N, typename T>
  std::string ToString(const QGenericMatrix<M,N,T>& in);

  std::ostream& operator<<(std::ostream& out, QGenericMatrix<2,2,qreal> m);
  // 1-column, 2-rows (column vector)
  std::ostream& operator<<(std::ostream& out, QGenericMatrix<1,2,qreal> m);
  // 2-column, 1-rows (row vector)
  std::ostream& operator<<(std::ostream& out, QGenericMatrix<2,1,qreal> m);

} // viewer


#endif // __imageviewer_h_

// Configure (x)emacs for this file ...
// Local Variables:
// mode:c++
// compile-command: "make -C .. -k"
// End:
