/*
 * Copyright (c) 2015, Georgia Tech Research Corporation
 * All rights reserved.
 *
 * Author(s): Michael X. Grey <mxgrey@gatech.edu>
 *
 * Georgia Tech Graphics Lab and Humanoid Robotics Lab
 *
 * Directed by Prof. C. Karen Liu and Prof. Mike Stilman
 * <karenliu@cc.gatech.edu> <mstilman@cc.gatech.edu>
 *
 * This file is provided under the following "BSD-style" License:
 *   Redistribution and use in source and binary forms, with or
 *   without modification, are permitted provided that the following
 *   conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *   CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 *   USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 *   AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *   POSSIBILITY OF SUCH DAMAGE.
 */

#include <osgGA/GUIEventAdapter>

#include "osgDart/DefaultEventHandler.h"
#include "osgDart/Viewer.h"
#include "osgDart/render/ShapeNode.h"
#include "osgDart/EntityNode.h"
#include "osgDart/utils.h"

#include "dart/dynamics/Entity.h"


#include <iostream>

namespace osgDart
{

DefaultEventHandler::DefaultEventHandler(Viewer* _viewer)
  : mViewer(_viewer)
{
  mViewer->addInstructionText("Spacebar:     Turn simulation on/off for any active worlds\n");
  mViewer->addInstructionText("Ctrl+H:       Turn headlights on/off\n");

  for(size_t i=0; i<NUM_MOUSE_BUTTONS; ++i)
    for(size_t j=0; j<BUTTON_NOTHING; ++j)
      mSuppressButtonPicks[i][j] = false;
  mSuppressMovePicks = false;

  clearButtonEvents();
}

//==============================================================================
DefaultEventHandler::~DefaultEventHandler()
{
  // Do nothing
}

//==============================================================================
MouseButtonEvent DefaultEventHandler::getButtonEvent(MouseButton button) const
{
  return mLastButtonEvent[button];
}

//==============================================================================
double DefaultEventHandler::getWindowCursorX() const
{
  return mLastCursorPosition[0];
}

//==============================================================================
double DefaultEventHandler::getWindowCursorY() const
{
  return mLastCursorPosition[1];
}

//==============================================================================
Eigen::Vector3d DefaultEventHandler::getDeltaCursor(
    const Eigen::Vector3d& _fromPosition) const
{
  osg::Vec3d eye, center, up;
  mViewer->getCamera()->getViewMatrixAsLookAt(eye, center, up);
  Eigen::Vector3d n = osgToEigVec3(center - eye);

  Eigen::Vector3d near, far;
  getNearAndFarPointUnderCursor(near, far);
  Eigen::Vector3d v = far-near;

  double s = n.dot(_fromPosition - near) / n.dot(v);

  return near - _fromPosition + s*v;
}

//==============================================================================
void DefaultEventHandler::getNearAndFarPointUnderCursor(Eigen::Vector3d& near,
                                                        Eigen::Vector3d& far,
                                                        double distance) const
{
  osg::Camera* C = mViewer->getCamera();
  osg::Matrix VPW = C->getViewMatrix() * C->getProjectionMatrix()
      * C->getViewport()->computeWindowMatrix();
  osg::Matrix invVPW;
  invVPW.invert(VPW);

  double x = getWindowCursorX(), y = getWindowCursorY();
  osg::Vec3 osgNear = osg::Vec3(x,y,0.0) * invVPW;
  osg::Vec3 osgFar = osg::Vec3(x,y,distance) * invVPW;

  near = osgToEigVec3(osgNear);
  far = osgToEigVec3(osgFar);
}

//==============================================================================
const std::vector<PickInfo>& DefaultEventHandler::getButtonPicks(
    MouseButton button, MouseButtonEvent event) const
{
  if(BUTTON_NOTHING == event)
    return mMovePicks;

  return mButtonPicks[button][event];
}

//==============================================================================
const std::vector<PickInfo>& DefaultEventHandler::getMovePicks() const
{
  return mMovePicks;
}

//==============================================================================
void DefaultEventHandler::suppressButtonPicks(MouseButton button,
                                              MouseButtonEvent event)
{
  if(BUTTON_NOTHING == event)
    mSuppressMovePicks = true;
  else
    mSuppressButtonPicks[button][event] = true;
}

//==============================================================================
void DefaultEventHandler::suppressMovePicks()
{
  mSuppressMovePicks = true;
}

//==============================================================================
void DefaultEventHandler::activateButtonPicks(MouseButton button,
                                              MouseButtonEvent event)
{
  if(BUTTON_NOTHING == event)
    mSuppressMovePicks = false;
  else
    mSuppressButtonPicks[button][event] = false;
}

//==============================================================================
void DefaultEventHandler::activateMovePicks()
{
  mSuppressMovePicks = false;
}

//==============================================================================
void DefaultEventHandler::pick(std::vector<PickInfo>& infoVector,
                               const osgGA::GUIEventAdapter& ea)
{
  osgUtil::LineSegmentIntersector::Intersections hlist;

  infoVector.clear();
  if(mViewer->computeIntersections(ea, hlist))
  {
    infoVector.reserve(hlist.size());
    for(const osgUtil::LineSegmentIntersector::Intersection& intersect : hlist)
    {
      osg::Drawable* drawable = intersect.drawable;
      render::ShapeNode* shape =
          dynamic_cast<render::ShapeNode*>(drawable->getParent(0));
      if(shape)
      {
        PickInfo info;
        info.shape = shape->getShape();
        info.entity = shape->getParentEntityNode()->getEntity();
        info.normal = osgToEigVec3(intersect.getWorldIntersectNormal());
        info.position = osgToEigVec3(intersect.getWorldIntersectPoint());

        infoVector.push_back(info);
      }
    }
  }
}

//==============================================================================
static bool wasActive(MouseButtonEvent event)
{
  return ( (event == BUTTON_PUSH) || (event == BUTTON_DRAG) );
}

//==============================================================================
static void assignEventToButtons(
    MouseButtonEvent (&mLastButtonEvent)[NUM_MOUSE_BUTTONS],
    const osgGA::GUIEventAdapter& ea)
{
  MouseButtonEvent event;
  if(ea.getEventType() == osgGA::GUIEventAdapter::PUSH)
    event = BUTTON_PUSH;
  else if(ea.getEventType() == osgGA::GUIEventAdapter::DRAG)
    event = BUTTON_DRAG;
  else if(ea.getEventType() == osgGA::GUIEventAdapter::RELEASE)
    event = BUTTON_RELEASE;

  if(BUTTON_RELEASE == event)
  {
    if( (ea.getButtonMask() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) == 0
        && wasActive(mLastButtonEvent[LEFT_MOUSE]) )
      mLastButtonEvent[LEFT_MOUSE] = event;

    if( (ea.getButtonMask() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON) == 0
        && wasActive(mLastButtonEvent[RIGHT_MOUSE]) )
      mLastButtonEvent[RIGHT_MOUSE] = event;

    if( (ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON) == 0
        && wasActive(mLastButtonEvent[MIDDLE_MOUSE]) )
      mLastButtonEvent[MIDDLE_MOUSE] = event;
  }
  else
  {
    if(ea.getButtonMask() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
      mLastButtonEvent[LEFT_MOUSE] = event;

    if(ea.getButtonMask() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
      mLastButtonEvent[RIGHT_MOUSE] = event;

    if(ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
      mLastButtonEvent[MIDDLE_MOUSE] = event;
  }
}

//==============================================================================
bool DefaultEventHandler::handle(const osgGA::GUIEventAdapter& ea,
                                 osgGA::GUIActionAdapter&)
{
  switch(ea.getEventType())
  {
    case osgGA::GUIEventAdapter::PUSH:
    case osgGA::GUIEventAdapter::DRAG:
    case osgGA::GUIEventAdapter::RELEASE:
    case osgGA::GUIEventAdapter::MOVE:
      mLastCursorPosition[0] = ea.getX();
      mLastCursorPosition[1] = ea.getY();

      break;

    default:
      break;
  }

  switch(ea.getEventType())
  {
    case osgGA::GUIEventAdapter::KEYDOWN:
    {
      switch(ea.getKey())
      {
        case 8: // ctrl+h
        {
          mViewer->switchHeadlights(!mViewer->checkHeadlights());
          return true;
          break;
        }

        case ' ':
        {
          mViewer->simulate(!mViewer->isSimulating());
          return true;
          break;
        }
      }
    }

    case osgGA::GUIEventAdapter::MOVE:
    {
//      clearButtonEvents();
      if(!mSuppressMovePicks)
        pick(mMovePicks, ea);
      break;
    }

    case osgGA::GUIEventAdapter::PUSH:
    case osgGA::GUIEventAdapter::DRAG:
    case osgGA::GUIEventAdapter::RELEASE:

      assignEventToButtons(mLastButtonEvent, ea);
      eventPick(ea);
      break;

    default:
      break;
  }

  return false;
}

//==============================================================================
void DefaultEventHandler::eventPick(const osgGA::GUIEventAdapter& ea)
{
  MouseButtonEvent mbe;
  switch(ea.getEventType())
  {
    case osgGA::GUIEventAdapter::PUSH:
      mbe = BUTTON_PUSH;
      break;
    case osgGA::GUIEventAdapter::DRAG:
      mbe = BUTTON_DRAG;
      break;
    case osgGA::GUIEventAdapter::RELEASE:
      mbe = BUTTON_RELEASE;
      break;
    default:
      return;
  }

  if(   ( (ea.getButtonMask() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
           && !mSuppressButtonPicks[LEFT_MOUSE][mbe])
     || ( (ea.getButtonMask() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
           && !mSuppressButtonPicks[RIGHT_MOUSE][mbe])
     || ( (ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
           && !mSuppressButtonPicks[MIDDLE_MOUSE][mbe]))
  {
    pick(mTempPicks, ea);

    if(ea.getButtonMask() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
      mButtonPicks[LEFT_MOUSE][mbe] = mTempPicks;

    if(ea.getButtonMask() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
      mButtonPicks[RIGHT_MOUSE][mbe] = mTempPicks;

    if(ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
      mButtonPicks[MIDDLE_MOUSE][mbe] = mTempPicks;
  }
}

//==============================================================================
void DefaultEventHandler::clearButtonEvents()
{
  for(size_t i=0; i<NUM_MOUSE_BUTTONS; ++i)
    mLastButtonEvent[i] = BUTTON_NOTHING;
}

} // namespace osgDart