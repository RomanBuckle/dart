/*
  RTQL8, Copyright (c) 2011 Georgia Tech Graphics Lab
  All rights reserved.

  Author        Sehoon Ha
  Date      06/07/2011
*/

#ifndef MODEL3D_BODYNODE_H
#define MODEL3D_BODYNODE_H

#include <vector>
#include <Eigen/Dense>
#include "renderer/RenderInterface.h"
#include "utils/EigenHelper.h"

namespace model3d {
#define MAX_NODE3D_NAME 128

    class Marker;
    class Dof;
    class Transformation;
    class Primitive;
    class Skeleton;
    class Joint;

    /**
       @brief BodyNode class represents a single node of the skeleton.

       BodyNode is a basic element of the skeleton. BodyNodes are hierarchically
       connected and have a set of core functions for calculating derivatives.
       Mostly automatically constructed by FileInfoSkel. @see FileInfoSkel.
    */
    class BodyNode {
    public:      
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW // we need this aligned allocator because we have Matrix4d as members in this class

        BodyNode(const char *_name = NULL); ///< Default constructor. The name can be up to 128
        virtual ~BodyNode(); ///< Default destructor

        // transformations
        Eigen::Matrix4d mT; ///< Local transformation from parent to itself
        Eigen::Matrix4d mW; ///< Global transformation
        Eigen::Matrix3d mIc;    ///< Inertia matrix in the world frame = R*Ibody*RT; updated by evalTransform

        // first derivatives
        EIGEN_V_MAT4D mTq;  ///< Partial derivative of local transformation wrt local dofs; each element is a 4x4 matrix
        EIGEN_V_MAT4D mWq;  ///< Partial derivative of world transformation wrt all dependent dofs; each element is a 4x4 matrix
        Eigen::MatrixXd mJv; ///< Linear Jacobian; Cartesian_linear_velocity of the COM = mJv * generalized_velocity
        Eigen::MatrixXd mJw; ///< Angular Jacobian; Cartesian_angular_velocity = mJw * generalized_velocity

        void init(); ///< Initialize the vector memebers with proper sizes
        void updateTransform(); ///< Update transformations w.r.t. the current dof values in Dof*
        void updateFirstDerivatives();  ///< Update the first derivatives of the transformations 
        void evalJacLin(); ///< Evaluate linear Jacobian of this body node (num cols == num dependent dofs)
        void evalJacAng(); ///< Evaluate angular Jacobian of this body node (num cols == num dependent dofs)

        inline Eigen::Matrix4d getWorldTransform() const { return mW; } ///< Transformation from the local coordinates of this body node to the world coordinates
        inline Eigen::Matrix4d getWorldInvTransform() const { return mW.inverse(); } ///< Transformation from the world coordinates to the local coordinates of this body node
        inline Eigen::Matrix4d getLocalTransform() const { return mT; } ///< Transformation from the local coordinates of this body node to the local coordinates of its parent
        inline Eigen::Matrix4d getLocalInvTransform() const { return mT.inverse(); } ///< Transformation from the local coordinates of the parent node to the local coordinates of this body node

        Eigen::Vector3d evalWorldPos(const Eigen::Vector3d& _lp); ///< Given a 3D vector lp in the local coordinates of this body node, return the world coordinates of this vector

        Eigen::Matrix4d getLocalDeriv(Dof *_q) const;

        /* void setDependDofMap(int _numDofs); ///< set up the dof dependence map for this node */
        /* bool dependsOn(int _dofIndex) const { return mDependsOnDof[_dofIndex]; } ///< NOTE: not checking index range */
        void setDependDofList(); ///< Set up the list of dependent dofs 
        bool dependsOn(int _dofIndex) const; ///< Test whether this dof is dependant or not \warning{You may want to use getNumDependantDofs / getDependantDof for efficiency}
        inline int getNumDependantDofs() const { return mDependantDofs.size(); } ///< The number of the dofs which this node is affected
        inline int getDependantDof(int _arrayIndex) { return mDependantDofs[_arrayIndex]; } ///< Return an dof index from the array index (< getNumDependantDofs)

        void draw(renderer::RenderInterface* _ri = NULL, const Eigen::Vector4d& _color = Eigen::Vector4d::Ones(), bool _useDefaultColor = true, int _depth = 0) const ;    ///< Render the entire subtree rooted at this body node
        void drawHandles(renderer::RenderInterface* _ri = NULL, const Eigen::Vector4d& _color = Eigen::Vector4d::Ones(), bool _useDefaultColor = true) const ;    ///< Render the handles

        inline char* getName() { return mName; }

        inline void setLocalCOM(const Eigen::Vector3d& _off) { mCOMLocal = _off; }
        inline Eigen::Vector3d getLocalCOM() const { return mCOMLocal; }
        
        inline void setSkel(Skeleton* _skel) { mSkel = _skel; }
        inline Skeleton* getSkel() const { return mSkel; }

        inline void setSkelIndex(int _idx) { mSkelIndex = _idx; }
        inline int getSkelIndex() const { return mSkelIndex; }
        
        inline BodyNode* getParentNode() const { return mNodeParent; }
        inline double getMass() const { return mMass; }

        inline void addHandle(Marker *_h) { mHandles.push_back(_h); }
        inline int getNumHandles() const { return mHandles.size(); }
        inline Marker* getHandle(int _idx) const { return mHandles[_idx]; }

        inline void setPrimitive(Primitive *_p) { mPrimitive = _p; }
        inline Primitive* getPrimitive() const { return mPrimitive; }
        
        inline void addChildJoint(Joint *_c) { mJointsChild.push_back(_c); }
        inline int getNumChildJoints() { return mJointsChild.size(); }
        inline Joint* getChildJoint(int _idx) const { return mJointsChild[_idx]; }
        inline Joint* getParentJoint() const { return mJointParent; }
        void setParentJoint(Joint *_p);

        // wrapper functions for joints
        BodyNode* getChildNode(int _idx) const;
        int getNumLocalDofs() const;
        Dof* getDof(int _idx) const;
        bool isPresent(Dof *_q);

    protected:
        
        char mName[MAX_NODE3D_NAME]; ///< Name
        int mSkelIndex;    ///< Index in the model

        Primitive *mPrimitive;  ///< Geometry of this body node
        std::vector<Joint *> mJointsChild; ///< List of joints that link to child nodes
        Joint *mJointParent;    ///< Joint connecting to parent node
        BodyNode *mNodeParent;      ///< Parent node
        std::vector<Marker *> mHandles; ///< List of handles associated

        std::vector<int> mDependantDofs; ///< A list of dependant dof indices 
        int mNumRootTrans;  ///< keep track of the root translation DOFs only if they are the first ones

        double mMass; ///< Mass of this node; zero if no primitive
        Eigen::Vector3d mCOMLocal; ///< COM of this body node in its local coordinate frame
        Skeleton *mSkel; ///< Pointer to the model this body node belongs to

    private:
        int mID; ///< A unique ID of this node globally 
        static int msBodyNodeCount; ///< Counts the number of nodes globally
    };

} // namespace model3d

#endif // #ifndef MODEL3D_BODYNODE_H

