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

#include <iostream>
#include <gtest/gtest.h>
#include "TestHelpers.h"

#include "dart/math/Geometry.h"
#include "dart/utils/SkelParser.h"
#include "dart/dynamics/BodyNode.h"
#include "dart/dynamics/RevoluteJoint.h"
#include "dart/dynamics/Skeleton.h"
#include "dart/simulation/World.h"

using namespace dart;
using namespace math;
using namespace dynamics;
using namespace simulation;

void check_self_consistency(SkeletonPtr skeleton)
{
  for(size_t i=0; i<skeleton->getNumBodyNodes(); ++i)
  {
    BodyNode* bn = skeleton->getBodyNode(i);
    EXPECT_TRUE(bn->getIndexInSkeleton() == i);
    EXPECT_TRUE(skeleton->getBodyNode(bn->getName()) == bn);

    Joint* joint = bn->getParentJoint();
    EXPECT_TRUE(skeleton->getJoint(joint->getName()) == joint);

    for(size_t j=0; j<joint->getNumDofs(); ++j)
    {
      DegreeOfFreedom* dof = joint->getDof(j);
      EXPECT_TRUE(dof->getIndexInJoint() == j);
      EXPECT_TRUE(skeleton->getDof(dof->getName()) == dof);
    }
  }

  for(size_t i=0; i<skeleton->getNumDofs(); ++i)
  {
    DegreeOfFreedom* dof = skeleton->getDof(i);
    EXPECT_TRUE(dof->getIndexInSkeleton() == i);
    EXPECT_TRUE(skeleton->getDof(dof->getName()) == dof);
  }
}

void constructSubtree(std::vector<BodyNode*>& _tree, BodyNode* bn)
{
  _tree.push_back(bn);
  for(size_t i=0; i<bn->getNumChildBodyNodes(); ++i)
    constructSubtree(_tree, bn->getChildBodyNode(i));
}

TEST(Skeleton, Restructuring)
{
  std::vector<std::string> fileList;
  fileList.push_back(DART_DATA_PATH"skel/test/chainwhipa.skel");
  fileList.push_back(DART_DATA_PATH"skel/test/single_pendulum.skel");
  fileList.push_back(DART_DATA_PATH"skel/test/single_pendulum_euler_joint.skel");
  fileList.push_back(DART_DATA_PATH"skel/test/single_pendulum_ball_joint.skel");
  fileList.push_back(DART_DATA_PATH"skel/test/double_pendulum.skel");
  fileList.push_back(DART_DATA_PATH"skel/test/double_pendulum_euler_joint.skel");
  fileList.push_back(DART_DATA_PATH"skel/test/double_pendulum_ball_joint.skel");
  fileList.push_back(DART_DATA_PATH"skel/test/serial_chain_revolute_joint.skel");
  fileList.push_back(DART_DATA_PATH"skel/test/serial_chain_eulerxyz_joint.skel");
  fileList.push_back(DART_DATA_PATH"skel/test/serial_chain_ball_joint.skel");
  fileList.push_back(DART_DATA_PATH"skel/test/serial_chain_ball_joint_20.skel");
  fileList.push_back(DART_DATA_PATH"skel/test/serial_chain_ball_joint_40.skel");
  fileList.push_back(DART_DATA_PATH"skel/test/simple_tree_structure.skel");
  fileList.push_back(DART_DATA_PATH"skel/test/simple_tree_structure_euler_joint.skel");
  fileList.push_back(DART_DATA_PATH"skel/test/simple_tree_structure_ball_joint.skel");
  fileList.push_back(DART_DATA_PATH"skel/test/tree_structure.skel");
  fileList.push_back(DART_DATA_PATH"skel/test/tree_structure_euler_joint.skel");
  fileList.push_back(DART_DATA_PATH"skel/test/tree_structure_ball_joint.skel");
  fileList.push_back(DART_DATA_PATH"skel/fullbody1.skel");

  std::vector<WorldPtr> worlds;
  for(size_t i=0; i<fileList.size(); ++i)
    worlds.push_back(dart::utils::SkelParser::readWorld(fileList[i]));

  std::vector<SkeletonPtr> skeletons;
  for(size_t i=0; i<worlds.size(); ++i)
  {
    WorldPtr world = worlds[i];
    for(size_t j=0; j<world->getNumSkeletons(); ++j)
      skeletons.push_back(world->getSkeleton(j));
  }

#ifndef NDEBUG
  size_t numIterations = 10;
#else
  size_t numIterations = 2*skeletons.size();
#endif

  // Test moves within the current Skeleton
  for(size_t i=0; i<numIterations; ++i)
  {
    size_t index = floor(math::random(0, skeletons.size()));
    index = std::min(index, skeletons.size()-1);
    SkeletonPtr skeleton = skeletons[index];
    SkeletonPtr original = skeleton->clone();

    size_t maxNode = skeleton->getNumBodyNodes()-1;
    BodyNode* bn1 = skeleton->getBodyNode(floor(math::random(0, maxNode)));
    BodyNode* bn2 = skeleton->getBodyNode(ceil(math::random(0, maxNode)));

    if(bn1 == bn2)
    {
      --i;
      continue;
    }

    BodyNode* child = bn1->descendsFrom(bn2)? bn1 : bn2;
    BodyNode* parent = child == bn1? bn2 : bn1;

    child->moveTo(parent);

    EXPECT_TRUE(skeleton->getNumBodyNodes() == original->getNumBodyNodes());
    if(skeleton->getNumBodyNodes() == original->getNumBodyNodes())
    {
      for(size_t j=0; j<skeleton->getNumBodyNodes(); ++j)
      {
        // Make sure no BodyNodes have been lost or gained in translation
        std::string name = original->getBodyNode(j)->getName();
        BodyNode* bn = skeleton->getBodyNode(name);
        EXPECT_FALSE(bn == nullptr);
        if(bn)
          EXPECT_TRUE(bn->getName() == name);

        name = skeleton->getBodyNode(j)->getName();
        bn = original->getBodyNode(name);
        EXPECT_FALSE(bn == nullptr);
        if(bn)
          EXPECT_TRUE(bn->getName() == name);


        // Make sure no Joints have been lost or gained in translation
        name = original->getJoint(j)->getName();
        Joint* joint = skeleton->getJoint(name);
        EXPECT_FALSE(joint == nullptr);
        if(joint)
          EXPECT_TRUE(joint->getName() == name);

        name = skeleton->getJoint(j)->getName();
        joint = original->getJoint(name);
        EXPECT_FALSE(joint == nullptr);
        if(joint)
          EXPECT_TRUE(joint->getName() == name);
      }
    }

    EXPECT_TRUE(skeleton->getNumDofs() == original->getNumDofs());
    for(size_t j=0; j<skeleton->getNumDofs(); ++j)
    {
      std::string name = original->getDof(j)->getName();
      DegreeOfFreedom* dof = skeleton->getDof(name);
      EXPECT_FALSE(dof == nullptr);
      if(dof)
        EXPECT_TRUE(dof->getName() == name);

      name = skeleton->getDof(j)->getName();
      dof = original->getDof(name);
      EXPECT_FALSE(dof == nullptr);
      if(dof)
        EXPECT_TRUE(dof->getName() == name);
    }
  }

  // Test moves between Skeletons
  for(size_t i=0; i<numIterations; ++i)
  {
    size_t fromIndex = floor(math::random(0, skeletons.size()));
    fromIndex = std::min(fromIndex, skeletons.size()-1);
    SkeletonPtr fromSkel = skeletons[fromIndex];

    if(fromSkel->getNumBodyNodes() == 0)
    {
      --i;
      continue;
    }

    size_t toIndex = floor(math::random(0, skeletons.size()));
    toIndex = std::min(toIndex, skeletons.size()-1);
    SkeletonPtr toSkel = skeletons[toIndex];

    if(toSkel->getNumBodyNodes() == 0)
    {
      --i;
      continue;
    }

    BodyNode* childBn = fromSkel->getBodyNode(
          floor(math::random(0, fromSkel->getNumBodyNodes()-1)));
    BodyNode* parentBn = toSkel->getBodyNode(
          floor(math::random(0, toSkel->getNumBodyNodes()-1)));

    if(fromSkel == toSkel)
    {
      if(childBn == parentBn)
      {
        --i;
        continue;
      }

      if(parentBn->descendsFrom(childBn))
      {
        BodyNode* tempBn = childBn;
        childBn = parentBn;
        parentBn = tempBn;

        SkeletonPtr tempSkel = fromSkel;
        fromSkel = toSkel;
        toSkel = tempSkel;
      }
    }

    BodyNode* originalParent = childBn->getParentBodyNode();
    std::vector<BodyNode*> subtree;
    constructSubtree(subtree, childBn);

    // Move to a new Skeleton
    childBn->moveTo(parentBn);

    // Make sure all the objects have moved
    for(size_t j=0; j<subtree.size(); ++j)
    {
      BodyNode* bn = subtree[j];
      EXPECT_TRUE(bn->getSkeleton() == toSkel);
    }

    // Move to the Skeleton's root while producing a new Joint type
    sub_ptr<Joint> originalJoint = childBn->getParentJoint();
    childBn->moveTo<FreeJoint>(nullptr);

    // The original parent joint should be deleted now
    EXPECT_TRUE(originalJoint == nullptr);

    // The BodyNode should now be a root node
    EXPECT_TRUE(childBn->getParentBodyNode() == nullptr);

    // The subtree should still be in the same Skeleton
    for(size_t j=0; j<subtree.size(); ++j)
    {
      BodyNode* bn = subtree[j];
      EXPECT_TRUE(bn->getSkeleton() == toSkel);
    }

    // Create some new Skeletons and mangle them all up

    childBn->copyTo<RevoluteJoint>(fromSkel, originalParent);

    SkeletonPtr temporary = childBn->split("temporary");
    SkeletonPtr other_temporary =
        childBn->split<PrismaticJoint>("other temporary");
    SkeletonPtr another_temporary = childBn->copyAs("another temporary");
    SkeletonPtr last_temporary = childBn->copyAs<ScrewJoint>("last temporary");

    childBn->copyTo(another_temporary->getBodyNode(
                      another_temporary->getNumBodyNodes()-1));
    childBn->copyTo<PlanarJoint>(another_temporary->getBodyNode(0));
    childBn->copyTo<TranslationalJoint>(temporary, nullptr);
    childBn->moveTo(last_temporary,
        last_temporary->getBodyNode(last_temporary->getNumBodyNodes()-1));
    childBn->moveTo<BallJoint>(last_temporary, nullptr);
    childBn->moveTo<EulerJoint>(last_temporary,
                                last_temporary->getBodyNode(0));
    childBn->changeParentJointType<FreeJoint>();


    // Check that the mangled Skeletons are all self-consistent
    check_self_consistency(fromSkel);
    check_self_consistency(toSkel);
    check_self_consistency(temporary);
    check_self_consistency(other_temporary);
    check_self_consistency(another_temporary);
    check_self_consistency(last_temporary);
  }
}

TEST(Skeleton, Persistence)
{
  WeakBodyNodePtr weakBnPtr;
  SoftBodyNodePtr softBnPtr;
  WeakSoftBodyNodePtr weakSoftBnPtr;
  WeakSkeletonPtr weakSkelPtr;
  {
    BodyNodePtr strongPtr;
    {
      {
        SkeletonPtr skeleton = createThreeLinkRobot(
              Eigen::Vector3d(1.0, 1.0, 1.0), DOF_X,
              Eigen::Vector3d(1.0, 1.0, 1.0), DOF_Y,
              Eigen::Vector3d(1.0, 1.0, 1.0), DOF_Z);
        weakSkelPtr = skeleton;

        // Test usability of the BodyNodePtr
        strongPtr = skeleton->getBodyNode(0);
        weakBnPtr = strongPtr;
        ConstBodyNodePtr constPtr = strongPtr;

        EXPECT_FALSE( strongPtr == nullptr );
        EXPECT_FALSE( nullptr == strongPtr );

        EXPECT_TRUE( strongPtr == skeleton->getBodyNode(0) );
        EXPECT_TRUE( skeleton->getBodyNode(0) == strongPtr );
        EXPECT_TRUE( constPtr == strongPtr );
        EXPECT_TRUE( weakBnPtr.lock() == strongPtr );

        EXPECT_FALSE( strongPtr < constPtr );
        EXPECT_FALSE( strongPtr < skeleton->getBodyNode(0) );
        EXPECT_FALSE( strongPtr < weakBnPtr.lock() );
        EXPECT_FALSE( skeleton->getBodyNode(0) < strongPtr );
        EXPECT_FALSE( weakBnPtr.lock() < strongPtr);

        EXPECT_FALSE( strongPtr > constPtr );
        EXPECT_FALSE( strongPtr > skeleton->getBodyNode(0) );
        EXPECT_FALSE( strongPtr > weakBnPtr.lock() );
        EXPECT_FALSE( skeleton->getBodyNode(0) > strongPtr );
        EXPECT_FALSE( weakBnPtr.lock() > strongPtr );

        BodyNodePtr tail = skeleton->getBodyNode(skeleton->getNumBodyNodes()-1);
        std::pair<Joint*, SoftBodyNode*> pair =
            skeleton->createJointAndBodyNodePair<RevoluteJoint, SoftBodyNode>(
              tail);

        softBnPtr = pair.second;
        weakSoftBnPtr = softBnPtr;
        WeakBodyNodePtr otherWeakPtr = weakSoftBnPtr; // Test convertability

        // Test usability of the DegreeOfFreedomPtr
        DegreeOfFreedomPtr dof = skeleton->getDof(1);
        WeakDegreeOfFreedomPtr weakdof = dof;
        ConstDegreeOfFreedomPtr const_dof = dof;
        WeakConstDegreeOfFreedomPtr const_weakdof = weakdof;
        const_weakdof = const_dof;

        EXPECT_TRUE( dof == skeleton->getDof(1) );
        EXPECT_TRUE( dof == const_dof );
        EXPECT_TRUE( weakdof.lock() == const_weakdof.lock() );
        EXPECT_TRUE( const_weakdof.lock() == skeleton->getDof(1) );
        EXPECT_TRUE( skeleton->getDof(1) == const_weakdof.lock() );

        EXPECT_FALSE( dof < const_dof );
        EXPECT_FALSE( dof < skeleton->getDof(1) );
        EXPECT_FALSE( dof < weakdof.lock() );
        EXPECT_FALSE( skeleton->getDof(1) < dof );
        EXPECT_FALSE( weakdof.lock() < dof );

        EXPECT_FALSE( dof > const_dof );
        EXPECT_FALSE( dof > skeleton->getDof(1) );
        EXPECT_FALSE( dof > weakdof.lock() );
        EXPECT_FALSE( skeleton->getDof(1) > dof );
        EXPECT_FALSE( weakdof.lock() > dof );

        dof = nullptr;
        weakdof = nullptr;
        const_dof = nullptr;
        const_weakdof = nullptr;

        EXPECT_TRUE( dof == nullptr );
        EXPECT_TRUE( nullptr == dof );
        EXPECT_TRUE( weakdof.lock() == nullptr );
        EXPECT_TRUE( nullptr == weakdof.lock() );
        EXPECT_TRUE( const_dof == nullptr );
        EXPECT_TRUE( const_weakdof.lock() == nullptr );

        EXPECT_FALSE( dof < const_dof );

        // Test usability of the JointPtr
        JointPtr joint = skeleton->getJoint(1);
        WeakJointPtr weakjoint = joint;
        ConstJointPtr const_joint = joint;
        WeakConstJointPtr const_weakjoint = const_joint;

        EXPECT_TRUE( joint == skeleton->getJoint(1) );
        EXPECT_TRUE( joint == const_joint );
        EXPECT_TRUE( weakjoint.lock() == const_weakjoint.lock() );
        EXPECT_TRUE( const_weakjoint.lock() == skeleton->getJoint(1) );

        EXPECT_FALSE( joint < const_joint );
        EXPECT_FALSE( joint < skeleton->getJoint(1) );
        EXPECT_FALSE( joint < weakjoint.lock() );
        EXPECT_FALSE( skeleton->getJoint(1) < joint );
        EXPECT_FALSE( weakjoint.lock() < joint );

        EXPECT_FALSE( joint > const_joint );
        EXPECT_FALSE( joint > skeleton->getJoint(1) );
        EXPECT_FALSE( joint > weakjoint.lock() );
        EXPECT_FALSE( skeleton->getJoint(1) > joint );
        EXPECT_FALSE( weakjoint.lock() > joint );

        joint = nullptr;
        weakjoint = nullptr;
        const_joint = nullptr;
        const_weakjoint = nullptr;

        EXPECT_TRUE( joint == nullptr );
        EXPECT_TRUE( weakjoint.lock() == nullptr );
        EXPECT_TRUE( const_joint == nullptr );
        EXPECT_TRUE( const_weakjoint.lock() == nullptr );
      }

      // The BodyNode should still be alive, because a BodyNodePtr still
      // references it
      EXPECT_FALSE(weakBnPtr.expired());

      // The Skeleton should still be alive, because a BodyNodePtr still
      // references one of its BodyNodes
      EXPECT_FALSE(weakSkelPtr.lock() == nullptr);

      // Take the BodyNode out of its Skeleton and put it into a temporary one
      strongPtr->remove();

      // The BodyNode should still be alive, because a BodyNodePtr still
      // references it
      EXPECT_FALSE(weakBnPtr.expired());

      // The Skeleton should be destroyed, because it lost the only BodyNode
      // that still had a reference
      EXPECT_TRUE(weakSkelPtr.lock() == nullptr);

      // Update the weakSkelPtr so it references the Skeleton that still exists
      weakSkelPtr = strongPtr->getSkeleton();
      EXPECT_FALSE(weakSkelPtr.lock() == nullptr);

      // Change the BodyNode that this BodyNodePtr is referencing
      strongPtr = strongPtr->getChildBodyNode(0);

      // Make sure the Skeleton is still alive. If the SkeletonPtr being used
      // by the BodyNodePtr is not swapped atomically, then this will fail,
      // which means we cannot rely on BodyNodePtr to keep BodyNodes alive.
      EXPECT_FALSE(weakSkelPtr.lock() == nullptr);
    }

    SkeletonPtr other_skeleton = createThreeLinkRobot(
          Eigen::Vector3d(1.0, 1.0, 1.0), DOF_X,
          Eigen::Vector3d(1.0, 1.0, 1.0), DOF_Y,
          Eigen::Vector3d(1.0, 1.0, 1.0), DOF_Z);
    BodyNode* tail = other_skeleton->getBodyNode(
          other_skeleton->getNumBodyNodes()-1);

    WeakConstBodyNodePtr weakParentPtr;
    {
      ConstBodyNodePtr parent = strongPtr;
      parent = parent->getParentBodyNode();
      weakParentPtr = parent;
      strongPtr->moveTo(tail);

      // The Skeleton should still be alive because 'parent' exists
      EXPECT_FALSE(weakSkelPtr.lock() == nullptr);
    }

    // Now that 'parent' is out of scope, the Skeleton should be gone
    EXPECT_TRUE(weakSkelPtr.lock() == nullptr);
    EXPECT_TRUE(weakParentPtr.lock() == nullptr);

    weakBnPtr = strongPtr;
    weakSkelPtr = strongPtr->getSkeleton();
    EXPECT_FALSE(weakBnPtr.expired());
    EXPECT_FALSE(weakSkelPtr.expired());
  }

  // softBnPtr still exists, so it should be keeping the Skeleton active
  EXPECT_FALSE(weakBnPtr.expired());

  softBnPtr->remove();

  // Now that the SoftBodyNode which is holding the reference has been moved to
  // another Skeleton, the weakBnPtr and weakSkelPtr should disappear
  EXPECT_TRUE(weakBnPtr.expired());
  EXPECT_TRUE(weakSkelPtr.expired());

  // The WeakSoftBodyNodePtr should not have expired yet, because a strong
  // reference to its SoftBodyNode still exists
  EXPECT_FALSE(weakSoftBnPtr.expired());

  softBnPtr = nullptr;

  // Now that the SoftBodyNodePtr has been cleared, the WeakSoftBodyNodePtr
  // should also be cleared
  EXPECT_TRUE(weakSoftBnPtr.lock() == nullptr);
}

int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
