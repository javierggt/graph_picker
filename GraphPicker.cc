/**
   \file


   \author Javier Gonzalez
   \version $Id: TEMPLATE.cc.tpl,v 1.4 2003/09/25 14:38:19 lukas Exp $
   \date 29 Aug 2011
*/

static const char CVSId[] =
"$Id$";


#include "GraphPicker.h"

#include <QMouseEvent>
#include <QtWidgets>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <QGenericMatrix>

using namespace std;
using namespace picker;

namespace picker {

  // M columns, N rows (the opposite order of operator())
  template<int M, int N, typename T>
  string ToString(const QGenericMatrix<M,N,T>& in)
  {
    ostringstream out;
    out << "[";
    for (int row = 0; row != N; ++row) {
      if (row)
        out << " ";
      out << "[ " << in(row,0);
      for (int col = 1; col != M; ++col) {
        out << ", " << in(row,col);
      }
      out << "]";
      if (row != N-1)
        out << "\n";
    }
    out << "]" << endl;
    return out.str();
  }

  std::ostream& operator<<(std::ostream& out, QGenericMatrix<2,2,qreal> m)
  { return out << ToString(m); }
  std::ostream& operator<<(std::ostream& out, QGenericMatrix<1,2,qreal> m)
  { return out << ToString(m); }
  std::ostream& operator<<(std::ostream& out, QGenericMatrix<2,1,qreal> m)
  { return out << ToString(m); }
}


Matrix
InvertMatrix(const Matrix& in)
{
  qreal data[4] = {0,0,0,0};
  Matrix out(data);

  out(0,0) = in(1,1);
  out(0,1) = -in(0,1);
  out(1,0) = -in(1,0);
  out(1,1) = in(0,0);

  double det = in(0,0)*in(1,1) - in(1,0)*in(0,1);
  return out/det;
}

PointCollection::PointCollection(TopLayer* parent):
  fMarkerSize(10),
  fBrush(QColor(127, 0, 127)),
  fPen(Qt::SolidLine),
  fParent(parent)
{}


void
PointCollection::Paint(QPainter& painter)
{
  painter.setPen(fPen);
  painter.setBrush(fBrush);
  for (unsigned int i = 0; i != fPoints.size(); ++i) {
    Vector p = fParent->SizeMatrix()*fPoints[i].pos;
    Vector dxPlus = fParent->SizeMatrix()*fPoints[i].dxPlus;
    Vector dxMinus = fParent->SizeMatrix()*fPoints[i].dxMinus;
    Vector dyPlus = fParent->SizeMatrix()*fPoints[i].dyPlus;
    Vector dyMinus = fParent->SizeMatrix()*fPoints[i].dyMinus;

    painter.drawEllipse(p(0,0)-fMarkerSize/2,
                        p(1,0)-fMarkerSize/2,
                        fMarkerSize,
                        fMarkerSize);

    painter.drawLine(p(0,0),p(1,0),p(0,0) + dxPlus(0,0),p(1,0) + dxPlus(1,0));
    painter.drawLine(p(0,0),p(1,0),p(0,0) + dxMinus(0,0),p(1,0) + dxMinus(1,0));
    painter.drawLine(p(0,0),p(1,0),p(0,0) + dyPlus(0,0),p(1,0) + dyPlus(1,0));
    painter.drawLine(p(0,0),p(1,0),p(0,0) + dyMinus(0,0),p(1,0) + dyMinus(1,0));
  }
}


template<class PointType>
int
PointCollection::IntersectingIndex(PointType pos)
  const
{
  for (unsigned int i = 0; i != fPoints.size(); ++i) {
    QPointF p(fPoints[i].pos(0,0)*fParent->width() - pos.x(),
              fPoints[i].pos(1,0)*fParent->height() - pos.y());
    if (p.manhattanLength() < fMarkerSize/2) {
      return i;
    }
  }
  return -1;
}


template<class PointType>
bool
PointCollection::Intersects(PointType pos)
  const
{
  if (IntersectingIndex(pos) == -1)
    return false;
  return true;
}


template<class PointType>
Point&
PointCollection::GetPoint(PointType pos)
{
  const int i = IntersectingIndex(pos);
  if (i == -1) {
    cout << "Tried to get marker at an empty position" << endl;
    throw "Tried to get marker at an empty position";
  }
  return fPoints[i];
}


template<class PointType>
const Point&
PointCollection::GetPoint(PointType pos)
  const
{
  const int i = IntersectingIndex(pos);
  if (i == -1) {
    cout << "Tried to get marker at an empty position" << endl;
    throw "Tried to get marker at an empty position";
  }
  return fPoints[i];
}


void PointCollection::AddPoint(Vector pos)
{
  fPoints.push_back(Point(pos));
}


class CalibrateDialog: public QDialog {
public:
  CalibrateDialog(QWidget * parent = 0, Qt::WindowFlags flags = 0):
    QDialog(parent, flags)
  {
    QLayout* lay = new QVBoxLayout();
    x_field = new QLineEdit();
    y_field = new QLineEdit();
    lay->addWidget(x_field);
    lay->addWidget(y_field);
    QLayout* bottom = new QHBoxLayout();
    QPushButton* ok = new QPushButton("ok");
    bottom->addWidget(ok);
    QPushButton* cancel = new QPushButton("cancel");
    bottom->addWidget(cancel);
    lay->addItem(bottom);
    setLayout(lay);

    connect(ok, SIGNAL(pressed()), this, SLOT(accept()));
    connect(cancel, SIGNAL(pressed()), this, SLOT(reject()));
  }

  // QString's toDouble is dumb! Using atof instead.
  double x(){return atof(x_field->text().toStdString().c_str());}
  double y(){return atof(y_field->text().toStdString().c_str());}
private:
  QLineEdit* x_field;
  QLineEdit* y_field;
};


TopLayer::TopLayer(QWidget * parent):
  QWidget(parent),
  fIsCalibrated(false),
  fLogX(false),
  fLogY(false),
  fShowCrosshairs(false),
  fCustomXaxis(false),
  fCustomYaxis(false),
  fSettingAxis(false),
  fMousePressed(false),
  fPoints(this),
  fTargetBase(3),
  fPixelBase(3)
{
  fXaxis(0,0) = 1;
  fXaxis(1,0) = 0;
  fYaxis(0,0) = 0;
  fYaxis(1,0) = 1;

  ShowCrosshairs(fShowCrosshairs);
  setFocusPolicy(Qt::ClickFocus);
}


void
TopLayer::ResetCrosshairs()
{
  fXaxis(0,0) = 1;
  fXaxis(1,0) = 0;
  fYaxis(0,0) = 0;
  fYaxis(1,0) = 1;
  fCustomXaxis = false;
  fCustomYaxis = false;
}

void
TopLayer::ShowCrosshairs(bool show)
{
  setMouseTracking(show);
  fShowCrosshairs = show;
  update();
}

QPointF
MatrixToPoint(const Vector& m)
{
  return QPointF(m(0,0), m(1,0));
}


Vector
PointToMatrix(const QPointF& p)
{
  qreal d[2] = {0,0};
  d[0] = p.x();
  d[1] = p.y();
  Vector m(d);

  return m;
}


const Matrix&
TopLayer::SizeMatrix()
  const
{
  static qreal zero[4] = {0,0,0,0};
  static Matrix sizeMatrix(zero);
  sizeMatrix(0,0) = width();
  sizeMatrix(1,1) = height();
  return sizeMatrix;
}

const Matrix&
TopLayer::InvSizeMatrix()
  const
{
  static qreal zero[4] = {0,0,0,0};
  static Matrix sizeMatrix(zero);
  sizeMatrix(0,0) = 1./width();
  sizeMatrix(1,1) = 1./height();
  return sizeMatrix;
}


void
TopLayer::Reset()
{
  fCalibrationPointsImage.clear();
  fCalibrationPointsTarget.clear();

  fPixelToTarget = Matrix();
  fTargetToPixel = Matrix();
  fCalibToPixel  = Matrix();
  fPixelToCalib  = Matrix();

  fTargetBase = std::vector<Vector>(3);
  fPixelBase = std::vector<Vector>(3);
}


// void
// TopLayer::SetCalibrated(bool c)
// {
//   fIsCalibrated = c;
// }


Vector Pow10(const Vector& in, bool logx, bool logy)
{
  double a;
  if (logx)
    a = pow(10., in.x());
  else
    a = in.x();

  double b;
  if (logy)
    b = pow(10., in.y());
  else
    b = in.y();

  return Vector(a, b);
}


Vector Log10(const Vector& in, bool logx, bool logy)
{
  double a;
  if (logx)
    a = log10(in.x());
  else
    a = in.x();

  double b;
  if (logy)
    b = log10(in.y());
  else
    b = in.y();

  return Vector(a, b);
}

void
TopLayer::Calibrate()
{
  if (fCalibrationPointsImage.size() != 3)
    return;

  fPixelBase[0] = (fCalibrationPointsImage[0]);
  fPixelBase[1] = (fCalibrationPointsImage[1] -
                   fCalibrationPointsImage[0]);
  fPixelBase[2] = (fCalibrationPointsImage[2] -
                   fCalibrationPointsImage[0]);

  fTargetBase[0] = (Log10(fCalibrationPointsTarget[0], fLogX, fLogY));
  fTargetBase[1] = (Log10(fCalibrationPointsTarget[1], fLogX, fLogY) -
                    Log10(fCalibrationPointsTarget[0], fLogX, fLogY));
  fTargetBase[2] = (Log10(fCalibrationPointsTarget[2], fLogX, fLogY) -
                    Log10(fCalibrationPointsTarget[0], fLogX, fLogY));

  fCalibToPixel(0,0) = fPixelBase[1](0,0);
  fCalibToPixel(1,0) = fPixelBase[1](1,0);
  fCalibToPixel(0,1) = fPixelBase[2](0,0);
  fCalibToPixel(1,1) = fPixelBase[2](1,0);
  fPixelToCalib = InvertMatrix(fCalibToPixel);

  Matrix calibToTarget;
  calibToTarget(0,0) = fTargetBase[1](0,0);
  calibToTarget(1,0) = fTargetBase[1](1,0);
  calibToTarget(0,1) = fTargetBase[2](0,0);
  calibToTarget(1,1) = fTargetBase[2](1,0);
  Matrix targetToCalib = InvertMatrix(calibToTarget);

  fPixelToTarget = calibToTarget*fPixelToCalib;
  fTargetToPixel = fCalibToPixel*targetToCalib;

  // cout << "Calibration points (target)" << endl;
  // cout << "[ " << fCalibrationPointsTarget[0](0,0) << ", " << fCalibrationPointsTarget[0](1,0) << " ]" << endl;
  // cout << "[ " << fCalibrationPointsTarget[1](0,0) << ", " << fCalibrationPointsTarget[1](1,0) << " ]" << endl;
  // cout << "[ " << fCalibrationPointsTarget[2](0,0) << ", " << fCalibrationPointsTarget[2](1,0) << " ]" << endl;

  // cout << "Target base" << endl;
  // cout << "[ " << fTargetBase[0](0,0) << ", " << fTargetBase[0](1,0) << " ]" << endl;
  // cout << "[ " << fTargetBase[1](0,0) << ", " << fTargetBase[1](1,0) << " ]" << endl;
  // cout << "[ " << fTargetBase[2](0,0) << ", " << fTargetBase[2](1,0) << " ]" << endl;

  // cout << "Pixel base" << endl;
  // cout << "[ " << fPixelBase[0](0,0) << ", " << fPixelBase[0](1,0) << " ]" << endl;
  // cout << "[ " << fPixelBase[1](0,0) << ", " << fPixelBase[1](1,0) << " ]" << endl;
  // cout << "[ " << fPixelBase[2](0,0) << ", " << fPixelBase[2](1,0) << " ]" << endl;

  // cout << endl;

  // cout << "fCalibToPixel: " << endl
  //      << fCalibToPixel << endl
  //      << "fPixelToCalib: " << endl
  //      << fPixelToCalib << endl
  //      << " = " << endl
  //      << "fCalibToPixel*fPixelToCalib: " << endl
  //      << fCalibToPixel*fPixelToCalib << endl;

  // cout << "calibToTarget: " << endl
  //      << calibToTarget << endl
  //      << "targetToCalib: " << endl
  //      << targetToCalib << endl
  //      << "calibToTarget*targetToCalib: " << endl
  //      << calibToTarget*targetToCalib << endl;

  // cout << "fPixelToTarget: " << endl
  //      << fPixelToTarget << endl
  //      << "fTargetToPixel: " << endl
  //      << fTargetToPixel << endl;

  // cout << fTargetBase[0]  << endl
  //      << fPixelToTarget*fPixelBase[1] + fTargetBase[0]  << endl
  //      << fPixelToTarget*fPixelBase[2] + fTargetBase[0]  << endl;

  fIsCalibrated = true;
}


void
TopLayer::Clear()
{
  fPoints.Clear();
  update();
}


Vector
TopLayer::FromPixelToTarget(const Vector& point)
  const
{
  return fPixelToTarget*(point - fPixelBase[0]) + fTargetBase[0];
}


Vector
TopLayer::FromTargetToPixel(const Vector& point)
  const
{
  return fTargetToPixel*(point - fTargetBase[0]) + fPixelBase[0];
}


void
TopLayer::keyReleaseEvent(QKeyEvent* event)
{
  // if (fSettingAxis && event->key() == Qt::Key_Shift) {
  //   fSettingAxis = false;
  // }
}


void
TopLayer::mousePressEvent(QMouseEvent* event)
{

  if (event->modifiers() & Qt::ShiftModifier) {
    fStart = Vector(event->pos());
    fSettingAxis = true;
  }

  if (fPoints.Intersects(event->pos())) {
    fStart = SizeMatrix()*fPoints.GetPoint(event->pos()).pos;
    fMousePressed = true;
  }
}

void
TopLayer::mouseMoveEvent(QMouseEvent* event)
{
  fMousePos = event->pos();
  if (fMousePressed || fSettingAxis) {
    fEnd = Vector(event->pos());
    update();
  }
  update();
}

void
TopLayer::mouseReleaseEvent(QMouseEvent* event)
{

  if (fSettingAxis) {
    QPointF start(fStart.x(), fStart.y());
    QPointF end = event->pos();
    Vector v(end-start);
    v /= sqrt(v(0,0)*v(0,0) + v(1,0)*v(1,0));
    if (!fCustomXaxis) {
      fXaxis = InvSizeMatrix()*v;
      fCustomXaxis = true;
    }
    else if (!fCustomYaxis) {
      fYaxis = InvSizeMatrix()*v;
      fCustomYaxis = true;
    }
    fSettingAxis = false;
  }

  if (fMousePressed) {
    fEnd = Vector(event->pos());
    Vector proj = fPixelToTarget*InvSizeMatrix()*(fEnd-fStart);
    Point & point = fPoints.GetPoint(fStart);

    Vector xAxis = SizeMatrix()*fXaxis;
    Vector yAxis = SizeMatrix()*fYaxis;
    const double proj_0 = (fEnd-fStart)(0,0)*xAxis(0,0) + (fEnd-fStart)(1,0)*xAxis(1,0);
    const double proj_1 = -(fEnd-fStart)(0,0)*yAxis(0,0) + (fEnd-fStart)(1,0)*yAxis(1,0);

    if (abs(proj_0) > abs(proj_1)) {
      proj(1,0) = 0;
      if (proj(0,0)>0) {
        point.dxPlus = fTargetToPixel*proj;
      }
      else {
        point.dxMinus = fTargetToPixel*proj;
      }
    }
    else {
      proj(0,0) = 0;
      if (proj(1,0)>0) {
        point.dyPlus = fTargetToPixel*proj;
      }
      else {
        point.dyMinus = fTargetToPixel*proj;
      }
    }

  }
  fMousePressed = false;
}

void
TopLayer::mouseDoubleClickEvent(QMouseEvent* event)
{
  const int x = event->x();
  const int y = event->y();


  if (fCalibrationPointsImage.size() < 3) {
    CalibrateDialog dialog(this);
    int rc = dialog.exec();
    if (rc == QDialog::Accepted) {
      double u = dialog.x();
      double v = dialog.y();

      fCalibrationPointsImage.push_back(Vector(double(x)/width(),double(y)/height()));
      fCalibrationPointsTarget.push_back(Vector(u,v));
    }
    return;
  }

  if (!fIsCalibrated) {
    Calibrate();
  }

  if (fIsCalibrated) {
    fPoints.AddPoint(double(x)/width(), double(y)/height());
  }
  update();
}


void
TopLayer::paintEvent(QPaintEvent* event)
{
  QWidget::paintEvent(event);

  QPainter painter(this);
  fPoints.Paint(painter);
  if (fSettingAxis) {
    painter.drawLine(fStart(0,0), fStart(1,0),
                     fEnd(0,0), fEnd(1,0));
  }
  if (fMousePressed) {
    Vector xAxis = SizeMatrix()*fXaxis;
    Vector yAxis = SizeMatrix()*fYaxis;
    const double proj_0 = (fEnd-fStart)(0,0)*xAxis(0,0) + (fEnd-fStart)(1,0)*xAxis(1,0);
    const double proj_1 = -(fEnd-fStart)(0,0)*yAxis(0,0) + (fEnd-fStart)(1,0)*yAxis(1,0);

    Vector proj = fPixelToTarget*InvSizeMatrix()*(fEnd-fStart);
    if (abs(proj_0) > abs(proj_1))
      proj(1,0) = 0;
    else
      proj(0,0) = 0;

    Vector end = fStart + SizeMatrix()*fTargetToPixel*proj;
    painter.drawLine(fStart(0,0), fStart(1,0),
                     end(0,0), end(1,0));
  }

  if (fShowCrosshairs) {
    Vector xAxis = SizeMatrix()*fXaxis;
    Vector yAxis = SizeMatrix()*fYaxis;
    painter.drawLine(fMousePos - QPointF(xAxis(0,0), xAxis(1,0))*(fMousePos.x())/xAxis(0,0),
                     fMousePos + QPointF(xAxis(0,0), xAxis(1,0))*(width()-fMousePos.x())/xAxis(0,0));
    painter.drawLine(fMousePos - QPointF(yAxis(0,0), yAxis(1,0))*(fMousePos.y())/yAxis(1,0),
                     fMousePos + QPointF(yAxis(0,0), yAxis(1,0))*(height()-fMousePos.y())/yAxis(1,0));
  }
}


GraphPicker::GraphPicker(string filename):
  fImageLabel(new QLabel),
  fTopWidget(new TopLayer()),
  fScrollArea(new QScrollArea()),
  fToolbar(0),
  fFileMenu(0),
  fHelpMenu(0)
{
  QWidget* image = new QWidget();
  image->setAutoFillBackground(false);

  //setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

  fImageLabel->setBackgroundRole(QPalette::Base);
  fImageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  fImageLabel->setScaledContents(true);

  fTopWidget->setAutoFillBackground(false);

  QStackedLayout* stack_layout = new QStackedLayout();
  stack_layout->setStackingMode(QStackedLayout::StackAll);
  stack_layout->addWidget(fImageLabel);
  stack_layout->addWidget(fTopWidget);
  image->setLayout(stack_layout);


  fScrollArea->setBackgroundRole(QPalette::Dark);
  fScrollArea->setWidget(image);
  setCentralWidget(fScrollArea);
  fScrollArea->setWidgetResizable(true);

  createActions();
  //createMenus();
  createToolbar();

  setWindowTitle(tr("Graph Picker"));
  resize(500, 400);

  if (filename != "")
    open(filename);
}


void
GraphPicker::save()
{
  // PointCollection points = fTopWidget->GetPoints();
  // for (unsigned int i = 0; i != points.GetSize(); ++i) {
  //   Point& p = points.GetPoint(i);
  //   Vector v = fTopWidget->FromPixelToTarget(p.pos);
  //   cout << Pow10(v, fTopWidget->fLogX, fTopWidget->fLogY)(0,0) << "\t"
  //        << Pow10(v, fTopWidget->fLogX, fTopWidget->fLogY)(1,0) << "\t"
  //        << abs((Pow10(v + fTopWidget->PixelToTargetMatrix()*p.dxPlus, fTopWidget->fLogX, fTopWidget->fLogY) -
  //                Pow10(v, fTopWidget->fLogX, fTopWidget->fLogY))(0,0)) << "\t"
  //        << abs((Pow10(v + fTopWidget->PixelToTargetMatrix()*p.dxMinus, fTopWidget->fLogX, fTopWidget->fLogY) -
  //                Pow10(v, fTopWidget->fLogX, fTopWidget->fLogY))(0,0)) << "\t"
  //        << abs((Pow10(v + fTopWidget->PixelToTargetMatrix()*p.dyPlus, fTopWidget->fLogX, fTopWidget->fLogY) -
  //                Pow10(v, fTopWidget->fLogX, fTopWidget->fLogY))(1,0)) << "\t"
  //        << abs((Pow10(v + fTopWidget->PixelToTargetMatrix()*p.dyMinus, fTopWidget->fLogX, fTopWidget->fLogY) -
  //                Pow10(v, fTopWidget->fLogX, fTopWidget->fLogY))(1,0)) << "\n";
  // }
  // return;


  QString fileName = QFileDialog::getSaveFileName(this,
                                                  tr("Save File"), QDir::currentPath());
  if (!fileName.isEmpty()) {
    PointCollection points = fTopWidget->GetPoints();
    ofstream out(fileName.toStdString().c_str());
    for (unsigned int i = 0; i != points.GetSize(); ++i) {
      Point& p = points.GetPoint(i);
      Vector v = fTopWidget->FromPixelToTarget(p.pos);
      out << Pow10(v, fTopWidget->fLogX, fTopWidget->fLogY)(0,0) << "\t"
          << Pow10(v, fTopWidget->fLogX, fTopWidget->fLogY)(1,0) << "\t"
          << abs((Pow10(v + fTopWidget->PixelToTargetMatrix()*p.dxPlus, fTopWidget->fLogX, fTopWidget->fLogY) -
                  Pow10(v, fTopWidget->fLogX, fTopWidget->fLogY))(0,0)) << "\t"
          << abs((Pow10(v + fTopWidget->PixelToTargetMatrix()*p.dxMinus, fTopWidget->fLogX, fTopWidget->fLogY) -
                  Pow10(v, fTopWidget->fLogX, fTopWidget->fLogY))(0,0)) << "\t"
          << abs((Pow10(v + fTopWidget->PixelToTargetMatrix()*p.dyPlus, fTopWidget->fLogX, fTopWidget->fLogY) -
                  Pow10(v, fTopWidget->fLogX, fTopWidget->fLogY))(1,0)) << "\t"
          << abs((Pow10(v + fTopWidget->PixelToTargetMatrix()*p.dyMinus, fTopWidget->fLogX, fTopWidget->fLogY) -
                  Pow10(v, fTopWidget->fLogX, fTopWidget->fLogY))(1,0)) << "\n";
    }
    out.close();
  }
}


void
GraphPicker::open(string file)
{
  QString fileName;
  if (file == "")
    fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open File"), QDir::currentPath());
  else
    fileName = QString::fromStdString(file);

  if (!fileName.isEmpty()) {
    QImage image(fileName);
    if (image.isNull()) {
      QMessageBox::information(this, tr("Image Viewer"),
                               tr("Cannot load %1.").arg(fileName));
      return;
    }
    fImageLabel->setPixmap(QPixmap::fromImage(image));

    updateActions();
  }
}


// void
// GraphPicker::ToggleCalibrate()
// {
//   bool calibrated = calibrateAct->isChecked();
//   fTopWidget->SetCalibrated(calibrated);
// }


void
GraphPicker::Clear()
{
  fTopWidget->Clear();
}


void
GraphPicker::about()
{
  QMessageBox::about(this, tr("About Graph Picker"),
                     tr("<p>The <b>Graph Picker</b> program is a utility to convert plots"
                        "into text files."));
}


void
GraphPicker::SetLogY()
{
  bool logy = logYAct->isChecked();
  fTopWidget->SetLogY(logy);
}


void
GraphPicker::SetLogX()
{
  bool logx = logXAct->isChecked();
  fTopWidget->SetLogX(logx);
}


void
GraphPicker::createActions()
{
  openAct = new QAction(QIcon("/Users/javier/OneDrive/Delaware/GraphPicker/images/graph.png"), tr("&Open..."), this);
  openAct->setShortcut(tr("Ctrl+O"));
  connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

  saveAct = new QAction(QIcon("/Users/javier/OneDrive/Delaware/GraphPicker/images/Save.png"), tr("&Save..."), this);
  saveAct->setShortcut(tr("Ctrl+S"));
  connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

  exitAct = new QAction(QIcon("/Users/javier/OneDrive/Delaware/GraphPicker/images/Cancel.png"), tr("E&xit"), this);
  exitAct->setShortcut(tr("Ctrl+Q"));
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

  // logXAct = new QAction(QIcon("/Users/javier/OneDrive/Delaware/GraphPicker/images/log_x.png"), tr("log(&x)"), this);
  logXAct = new QAction(tr("log(&x)"), this);
  logXAct->setEnabled(true);
  logXAct->setCheckable(true);
  logXAct->setChecked(fTopWidget->fLogX);
  connect(logXAct, SIGNAL(triggered()), this, SLOT(SetLogX()));

  logYAct = new QAction(tr("log(&y)"), this);
  logYAct->setEnabled(true);
  logYAct->setCheckable(true);
  logYAct->setChecked(fTopWidget->fLogY);
  connect(logYAct, SIGNAL(triggered()), this, SLOT(SetLogY()));

  crossAct = new QAction(QIcon("/Users/javier/OneDrive/Delaware/GraphPicker/images/Cross_green.png"), tr("Show Crosshairs"), this);
  crossAct->setEnabled(true);
  crossAct->setCheckable(true);
  crossAct->setChecked(fTopWidget->fShowCrosshairs);
  connect(crossAct, SIGNAL(triggered(bool)), fTopWidget, SLOT(ShowCrosshairs(bool)));

  resetCrossAct = new QAction(QIcon("/Users/javier/OneDrive/Delaware/GraphPicker/images/Cross_red.png"), tr("Reset Crosshairs"), this);
  resetCrossAct->setEnabled(true);
  connect(resetCrossAct, SIGNAL(triggered()), fTopWidget, SLOT(ResetCrosshairs()));

  clearAct = new QAction(QIcon("/Users/javier/OneDrive/Delaware/GraphPicker/images/Trash.png"), tr("&Clear"), this);
  clearAct->setShortcut(tr("Ctrl+C"));
  connect(clearAct, SIGNAL(triggered()), this, SLOT(Clear()));

  // calibrateAct = new QAction(tr("&Calibrate"), this);
  // calibrateAct->setEnabled(true);
  // calibrateAct->setCheckable(true);
  // calibrateAct->setChecked(true);
  // calibrateAct->setShortcut(tr("Ctrl+C"));
  // connect(calibrateAct, SIGNAL(triggered()), this, SLOT(ToggleCalibrate()));

  aboutAct = new QAction(tr("&About"), this);
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  aboutQtAct = new QAction(tr("About &Qt"), this);
  connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

}


void
GraphPicker::createMenus()
{
  fFileMenu = new QMenu(tr("&File"), this);

  fFileMenu->addAction(openAct);
  fFileMenu->addAction(saveAct);
  fFileMenu->addSeparator();
  //fFileMenu->addAction(calibrateAct);
  fFileMenu->addAction(clearAct);
  fFileMenu->addAction(logXAct);
  fFileMenu->addAction(logYAct);
  fFileMenu->addAction(crossAct);
  fFileMenu->addAction(resetCrossAct);
  fFileMenu->addSeparator();
  fFileMenu->addAction(exitAct);

  fHelpMenu = new QMenu(tr("&Help"), this);
  fHelpMenu->addAction(aboutAct);
  fHelpMenu->addAction(aboutQtAct);

  menuBar()->addMenu(fFileMenu);
  menuBar()->addMenu(fHelpMenu);
}


void
GraphPicker::createToolbar()
{
  fToolbar = addToolBar(tr("Tools"));

  fToolbar->addAction(openAct);

  fToolbar->addAction(saveAct);
  // fToolbar->addAction(calibrateAct);
  fToolbar->addAction(clearAct);
  fToolbar->addSeparator();
  fToolbar->addAction(resetCrossAct);
  fToolbar->addAction(crossAct);

  // QToolButton* logXbutton = new QToolButton();
  // logXbutton->setFixedSize(QSize(48,24));
  // logXbutton->setDefaultAction(logXAct);
  // fToolbar->addWidget(logXbutton);
  fToolbar->addAction(logXAct);
  fToolbar->addAction(logYAct);
  fToolbar->addSeparator();
  fToolbar->addAction(exitAct);


}


void
GraphPicker::updateActions()
{
}


void
GraphPicker::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
  scrollBar->setValue(int(factor * scrollBar->value()
                          + ((factor - 1) * scrollBar->pageStep()/2)));
}


// Configure (x)emacs for this file ...
// Local Variables:
// mode:c++
// compile-command: "make -C .. -k"
// End:
