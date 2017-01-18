/*
 * Copyright (c) 2015-2016, Humanoid Lab, Georgia Tech Research Corporation
 * Copyright (c) 2015-2017, Graphics Lab, Georgia Tech Research Corporation
 * Copyright (c) 2016-2017, Personal Robotics Lab, Carnegie Mellon University
 * All rights reserved.
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

#ifndef DART_GUI_OSG_RENDER_PLANESHAPENODE_HPP_
#define DART_GUI_OSG_RENDER_PLANESHAPENODE_HPP_

#include <osg/MatrixTransform>

#include "dart/gui/osg/render/ShapeNode.hpp"

namespace dart {

namespace dynamics {
class PlaneShape;
} // namespace dynamics

namespace gui {
namespace osg {
namespace render {

class PlaneShapeGeode;
class PlaneShapeDrawable;

class PlaneShapeNode : public ShapeNode, public ::osg::Group
{
public:

  PlaneShapeNode(std::shared_ptr<dart::dynamics::PlaneShape> shape,
                 ShapeFrameNode* parent);

  void refresh();
  void extractData(bool firstTime);

protected:

  virtual ~PlaneShapeNode();

  std::shared_ptr<dart::dynamics::PlaneShape> mPlaneShape;
  PlaneShapeGeode* mGeode;

};

} // namespace render
} // namespace osg
} // namespace gui
} // namespace dart

#endif // DART_GUI_OSG_RENDER_PLANESHAPENODE_HPP_
