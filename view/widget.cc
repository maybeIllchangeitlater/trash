#include "widget.h"
#include <QResource>
#include <iostream>

OpenGLWidget::OpenGLWidget(s21::Settings &settings,
                           s21::Controller &controller, QWidget *parent)
    : settings_(settings),
      controller_(controller),
      QOpenGLWidget(parent), vbo_(QOpenGLBuffer::VertexBuffer), ibo_(QOpenGLBuffer::IndexBuffer) {
}

OpenGLWidget::~OpenGLWidget() { std::cout << "ustroy destroy" << std::endl; }

//void OpenGLWidget::SettingsChanged()
//{
//    shader_programm_.bind();

//    shader_programm_.release();
//}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    QPoint delta = event->pos() - last_mouse_pos_;
    settings_.rotation_x += delta.x();
    settings_.rotation_y += delta.y();
    update();
    last_mouse_pos_ = event->pos();
  } else if (event->buttons() & Qt::RightButton) {
    QPoint delta = event->pos() - last_rmouse_pos_;
    settings_.translation_x += delta.x() / 100.0f;
    settings_.translation_y -= delta.y() / 100.0f;
    update();
    last_rmouse_pos_ = event->pos();
  }
}


// void OpenGLWidget::ceterModel() {
//   translationZ = -m_medianZ * scaleBy;
//   translationX = -m_medianX * scaleBy;
//   translationY = -m_medianY * scaleBy;
//   update();
// }


void OpenGLWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) last_mouse_pos_ = event->pos();
  if (event->button() == Qt::RightButton) last_rmouse_pos_ = event->pos();
}

void OpenGLWidget::wheelEvent(QWheelEvent *event) {
  settings_.translation_z -= static_cast<float>(event->angleDelta().y()) / 120.0f;
  update();
}

void OpenGLWidget::initializeGL() {
  glClearColor(settings_.back_color.redF(), settings_.back_color.greenF(),
               settings_.back_color.blueF(), settings_.back_color.alphaF());
  glEnable(GL_DEPTH_TEST);
  /*----------------------------------------------------------------------------------------------------------------------*/
  controller_.ShaderVersion() == 1 ? shader_programm_.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/resources/VertexShader.txt") :
                                     shader_programm_.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/resources/VertexShaderCPU.txt");
  std::cout << "/t My shader version is :" << controller_.ShaderVersion() << std::endl;
  shader_programm_.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/resources/GeometryShader.txt");
  shader_programm_.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/resources/FragShader.txt");
  shader_programm_.link();
  shader_programm_.bind();
  /*----------------------------------------------------------------------------------------------------------------------*/
  shader_programm_.setUniformValue("color", settings_.color);
  /*----------------------------------------------------------------------------------------------------------------------*/

  vbo_.create();
  vbo_.bind();
  vbo_.allocate(controller_.GetVertexCopyConstRef().data(), controller_.GetVertexCopyConstRef().size() * sizeof(GLfloat));
  vbo_.setUsagePattern(QOpenGLBuffer::StaticDraw);

  ibo_.create();
  ibo_.bind();
  ibo_.allocate(controller_.GetFaceConstRef().data(), controller_.GetFaceConstRef().size() * sizeof(GLuint));
  ibo_.setUsagePattern(QOpenGLBuffer::StaticDraw);


  vao_.create();
  vao_.bind();
  int vertex_location = shader_programm_.attributeLocation("position"); // Get the attribute location from your shader program
  shader_programm_.enableAttributeArray(vertex_location);
  shader_programm_.setAttributeBuffer(vertex_location, GL_FLOAT, 0, 3);
  vbo_.release();
  vao_.release();
  ibo_.release();
  /*----------------------------------------------------------------------------------------------------------------------*/
  shader_programm_.release();
  /*----------------------------------------------------------------------------------------------------------------------*/
}

void OpenGLWidget::resizeGL(int w, int h) {
  /*----------------------------------------------------------------------------------------------------------------------*/
  glViewport(0, 0, w, h);
  projection_matrix_.setToIdentity();
  projection_matrix_.perspective(60.0f, static_cast<float>(w) / (h), 0.1f,
                                 100.0f);
  /*----------------------------------------------------------------------------------------------------------------------*/
}

void OpenGLWidget::paintGL() {
  /*----------------------------------------------------------------------------------------------------------------------*/
  glClearColor(settings_.back_color.redF(), settings_.back_color.greenF(),
               settings_.back_color.blueF(), settings_.back_color.alphaF());
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  shader_programm_.bind();
  shader_programm_.setUniformValue("projectionMatrix", projection_matrix_);
  controller_.MoveModel(model_view_matrix_, settings_);
  shader_programm_.setUniformValue("lineWidth", 5.0f);
  shader_programm_.setUniformValue("modelViewMatrix", model_view_matrix_); //add if
  shader_programm_.setUniformValue("color", settings_.color);

  vao_.bind();
  ibo_.bind();

  if(controller_.ShaderVersion() == 2){
      vbo_.bind();
      vbo_.allocate(controller_.GetVertexCopyConstRef().data(), controller_.GetVertexCopyConstRef().size() * sizeof(GLfloat));
      vbo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
      vbo_.release();
  }

  glDrawElements(GL_LINES, controller_.GetFaceConstRef().size(), GL_UNSIGNED_INT, nullptr);
  ibo_.release();
  vao_.release();
  shader_programm_.release();


}
