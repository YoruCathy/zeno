#pragma once

#include <zeno/zeno.h>
#include <zeno/logger.h>
#include <zeno/ListObject.h>
#include <zeno/NumericObject.h>
#include <zeno/PrimitiveObject.h>
#include <zeno/utils/UserData.h>
#include <zeno/StringObject.h>

#include <stable_anisotropic_NH.h>
#include <diriclet_damping.h>
#include <stable_isotropic_NH.h>
#include <bspline_isotropic_model.h>
#include <stable_Stvk.h>

#include <quasi_static_solver.h>
#include <backward_euler_integrator.h>
#include <example_based_quasi_static_solver.h>
#include <skin_driven_quasi_static_solver.h>
#include <fstream>
#include <algorithm>

#include <matrix_helper.hpp>
#include<Eigen/SparseCholesky>
#include <iomanip>

#include <cmath>

#include <time.h>

#include <Eigen/Geometry> 

namespace zeno {

struct MuscleModelObject : zeno::IObject {
    MuscleModelObject() = default;
    std::shared_ptr<BaseForceModel> _forceModel;
};

struct DampingForceModel : zeno::IObject {
    DampingForceModel() = default;
    std::shared_ptr<DiricletDampingModel> _dampForce;
};

struct FEMIntegrator : zeno::IObject {
    FEMIntegrator() = default;
    // the set of data structure which remain unchanged
    std::shared_ptr<BaseIntegrator> _intPtr;
    std::shared_ptr<MuscleModelObject> muscle;
    std::shared_ptr<DampingForceModel> damp;

    // std::vector<double> _elmYoungModulus;
    // std::vector<double> _elmPossonRatio;
    // std::vector<double> _elmDamp;
    // std::vector<double> _elmDensity;

    // std::vector<Vec3d> _elmFiberDir;

    // std::vector<double> _elmMass;
    // std::vector<double> _elmVolume;
    std::vector<Mat9x12d> _elmdFdx;
    std::vector<Mat4x4d> _elmMinv;

    std::vector<double> _elmCharacteristicNorm;

    SpMat _connMatrix;

    size_t _stepID;

    // initialize all the element-wise attributes by interpolating corresponding vertex-wise attributes, 
    // and assume all this attributes remain unchanged during the simulation process
    // the static parameters which remain unchanged include :
    //      1> Element-wise Elasto parameters : Young Modulus and Posson Ratio
    //      2> Element-wise Damping Coefficient
    //      3> Element-wise Density
    //      4> Element-wise Fiber Direction for muscle simulation
    //      5> Element-wise volume
    //      6> Topology information and other precomputed components for FEM simulation 
    void PrecomputeFEMInfo(const std::shared_ptr<zeno::PrimitiveObject>& prim){
        zeno::vec3f default_fdir = zeno::vec3f(1,0,0);
        size_t nm_elms = prim->quads.size();
        // _elmYoungModulus.resize(nm_elms);
        // _elmPossonRatio.resize(nm_elms);
        // _elmDamp.resize(nm_elms);
        // _elmDensity.resize(nm_elms);
        // _elmFiberDir.resize(nm_elms);

        // for(size_t elm_id = 0;elm_id < nm_elms;++elm_id){
        //     const auto& tet = prim->quads[elm_id];
        //     _elmDensity[elm_id] = 0;
        //     _elmYoungModulus[elm_id] = 0;
        //     _elmPossonRatio[elm_id] = 0;
        //     _elmDamp[elm_id] = 0;
        //     for(size_t i = 0;i < 4;++i){
        //         size_t idx = tet[i];
        //         _elmDensity[elm_id] += prim->attr<float>("phi")[idx];
        //         _elmYoungModulus[elm_id] += prim->attr<float>("E")[idx];
        //         _elmPossonRatio[elm_id] += prim->attr<float>("nu")[idx];
        //         _elmDamp[elm_id] += prim->attr<float>("dampingCoeff")[idx];
        //         zeno::vec3f fdir = prim->attr<zeno::vec3f>("fiberDir")[idx];
        //         Vec3d fdir_vec = Vec3d(fdir[0],fdir[1],fdir[2]);
        //         if(fdir_vec.norm() > 1e-5)
        //             fdir_vec /= fdir_vec.norm();

        //         _elmFiberDir[elm_id] += fdir_vec;
        //     }
        //     _elmDensity[elm_id] /= 4;
        //     _elmYoungModulus[elm_id] /= 4;
        //     _elmPossonRatio[elm_id] /= 4;
        //     _elmDamp[elm_id] /= 4;   
        //     _elmFiberDir[elm_id] /= _elmFiberDir[elm_id].norm();  
        // }
        // compute connectivity matrix
        std::set<Triplet,triplet_cmp> connTriplets;
        for (size_t elm_id = 0; elm_id < nm_elms; ++elm_id) {
            const auto& elm = prim->quads[elm_id];
            for (size_t i = 0; i < 4; ++i)
                for (size_t j = 0; j < 4; ++j)
                    for (size_t k = 0; k < 3; ++k)
                        for (size_t l = 0; l < 3; ++l) {
                            size_t row = elm[i] * 3 + k;
                            size_t col = elm[j] * 3 + l;
                            if(row > col)
                                continue;
                            if(row == col){
                                connTriplets.insert(Triplet(row, col, 1.0));
                            }else{
                                connTriplets.insert(Triplet(row, col, 1.0));
                                connTriplets.insert(Triplet(col, row, 1.0));
                            }
                        }
        }
        _connMatrix = SpMat(prim->size() * 3,prim->size() * 3);
        _connMatrix.setFromTriplets(connTriplets.begin(),connTriplets.end());
        _connMatrix.makeCompressed();

        // _elmVolume.resize(nm_elms);
        _elmdFdx.resize(nm_elms);
        _elmMinv.resize(nm_elms);
        _elmCharacteristicNorm.resize(nm_elms);
        for(size_t elm_id = 0;elm_id < nm_elms;++elm_id){
            auto elm = prim->quads[elm_id];
            Mat4x4d M;
            for(size_t i = 0;i < 4;++i){
                auto vert = prim->verts[elm[i]];
                M.block(0,i,3,1) << vert[0],vert[1],vert[2];
            }
            M.bottomRows(1).setConstant(1.0);
            // _elmVolume[elm_id] = fabs(M.determinant()) / 6;

            Mat3x3d Dm;
            for(size_t i = 1;i < 4;++i){
                auto vert = prim->verts[elm[i]];
                auto vert0 = prim->verts[elm[0]];
                Dm.col(i - 1) << vert[0]-vert0[0],vert[1]-vert0[1],vert[2]-vert0[2];
            }

            Mat3x3d DmInv = Dm.inverse();
            _elmMinv[elm_id] = M.inverse();

            double m = DmInv(0,0);
            double n = DmInv(0,1);
            double o = DmInv(0,2);
            double p = DmInv(1,0);
            double q = DmInv(1,1);
            double r = DmInv(1,2);
            double s = DmInv(2,0);
            double t = DmInv(2,1);
            double u = DmInv(2,2);

            double t1 = - m - p - s;
            double t2 = - n - q - t;
            double t3 = - o - r - u; 

            _elmdFdx[elm_id] << 
                t1, 0, 0, m, 0, 0, p, 0, 0, s, 0, 0, 
                0,t1, 0, 0, m, 0, 0, p, 0, 0, s, 0,
                0, 0,t1, 0, 0, m, 0, 0, p, 0, 0, s,
                t2, 0, 0, n, 0, 0, q, 0, 0, t, 0, 0,
                0,t2, 0, 0, n, 0, 0, q, 0, 0, t, 0,
                0, 0,t2, 0, 0, n, 0, 0, q, 0, 0, t,
                t3, 0, 0, o, 0, 0, r, 0, 0, u, 0, 0,
                0,t3, 0, 0, o, 0, 0, r, 0, 0, u, 0,
                0, 0,t3, 0, 0, o, 0, 0, r, 0, 0, u;

            Vec3d v0;v0 << prim->verts[elm[0]][0],prim->verts[elm[0]][1],prim->verts[elm[0]][2];
            Vec3d v1;v1 << prim->verts[elm[1]][0],prim->verts[elm[1]][1],prim->verts[elm[1]][2];
            Vec3d v2;v2 << prim->verts[elm[2]][0],prim->verts[elm[2]][1],prim->verts[elm[2]][2];
            Vec3d v3;v3 << prim->verts[elm[3]][0],prim->verts[elm[3]][1],prim->verts[elm[3]][2];

            FEM_Scaler A012 = MatHelper::Area(v0,v1,v2);
            FEM_Scaler A013 = MatHelper::Area(v0,v1,v3);
            FEM_Scaler A123 = MatHelper::Area(v1,v2,v3);
            FEM_Scaler A023 = MatHelper::Area(v0,v2,v3);

            // we denote the average surface area of a tet as the characteristic norm
            _elmCharacteristicNorm[elm_id] = (A012 + A013 + A123 + A023) / 4;
        }      
    }

    // Except for the static parameters set during the contruction process, you can manipulate
    // the dynamic parameters of FEM mesh in prim using wrangle.
    // Currently the supported dynamic parameters include:
    //      1>  examW               ------->    control the blending weight of example shape
    //      2>  examShape           ------->    Setting the example shape of the mesh
    //      3>  activation          ------->    Setting the vertex-wise activation level
    //      4> curPos               ------->    The current shape of the mesh
    void AssignElmAttribs(size_t elm_id,
            const std::shared_ptr<PrimitiveObject>& prim,
            const std::shared_ptr<PrimitiveObject>& elmView,
            TetAttributes& attrbs) const {
        attrbs._elmID = elm_id;
        attrbs._Minv = _elmMinv[elm_id];
        attrbs._dFdX = _elmdFdx[elm_id];

        const auto& tet = prim->quads[elm_id];

        // nodal wise properties
        // elm-wise properties
        // const auto& activation_field = prim->attr<zeno::vec3f>("activation");
        const auto& Es = elmView->attr<float>("E");
        const auto& nus = elmView->attr<float>("nu");
        const auto& vs = elmView->attr<float>("v");
        const auto& phis = elmView->attr<float>("phi");
        const auto& Vs = elmView->attr<float>("V");

        // Support for examShape-Driven Mechanics
        for(size_t i = 0;i < 4;++i){
            if(prim->has_attr("examShape") && prim->has_attr("examW")){
                const auto& example_shape = prim->attr<zeno::vec3f>("examShape");
                const auto& example_weight = prim->attr<float>("examW");
                attrbs._example_pos.segment(i*3,3) << example_shape[tet[i]][0],example_shape[tet[i]][1],example_shape[tet[i]][2];
                attrbs._example_pos_weight.segment(i*3,3).setConstant(example_weight[tet[i]]);
            }else{
                attrbs._example_pos.segment(i*3,3) << 0,0,0;
                attrbs._example_pos_weight.segment(i*3,3).setConstant(0);
            }
        }

        // Support non-uniform activation
        if(elmView->has_attr("activation") && elmView->has_attr("fiberDir")){
            const auto& activation_level = elmView->attr<zeno::vec3f>("activation");
            const auto& fiber_dir = elmView->attr<zeno::vec3f>("fiberDir");
            attrbs.emp.forient << fiber_dir[elm_id][0],fiber_dir[elm_id][1],fiber_dir[elm_id][2];
            Mat3x3d R = MatHelper::Orient2R(attrbs.emp.forient);
            Vec3d al;al << activation_level[elm_id][0],activation_level[elm_id][1],activation_level[elm_id][2];
            attrbs.emp.Act = R * al.asDiagonal() * R.transpose();
        }else{
            attrbs.emp.forient << 1,0,0;
            attrbs.emp.Act = Mat3x3d::Identity();
        }

        // Support Interpolatee-Driven Mechanics

        attrbs.emp.E = Es[elm_id];
        attrbs.emp.nu = nus[elm_id];
        attrbs.v = vs[elm_id];
        attrbs._volume = Vs[elm_id];
        attrbs._density = phis[elm_id];
    }

    void AssignElmInterpShape(size_t nm_elms,
        const std::shared_ptr<PrimitiveObject>& interpShape,
        std::vector<std::vector<Vec3d>>& interpPs,
        std::vector<std::vector<Vec3d>>& interpWs){
            interpPs.resize(nm_elms);
            interpWs.resize(nm_elms);
            for(size_t i = 0;i < nm_elms;++i){
                interpPs[i].clear();
                interpWs[i].clear();
            }
            // std::cout << "TRY ASSIGN INTERP SHAPE" << std::endl;
            if(interpShape && interpShape->has_attr("embed_id") && interpShape->has_attr("embed_w")){
                // std::cout << "ASSIGN ATTRIBUTES" << std::endl;
                const auto& elm_ids = interpShape->attr<float>("embed_id");
                const auto& elm_ws = interpShape->attr<zeno::vec3f>("embed_w");
                const auto& pos = interpShape->verts;

                // #pragma omp parallel for 
                for(size_t i = 0;i < interpShape->size();++i){
                    auto elm_id = elm_ids[i];
                    const auto& pos = interpShape->verts[i];
                    const auto& w = elm_ws[i];
                    interpPs[elm_id].emplace_back(pos[0],pos[1],pos[2]);
                    interpWs[elm_id].emplace_back(w[0],w[1],w[2]);
                }
            }
            // if(!interpShape){
            //     std::cout << "NULLPTR FOR INTERPSHAPE" << std::endl;
            // }
            // if()
    }

    FEM_Scaler EvalObj(const std::shared_ptr<PrimitiveObject>& shape,
        const std::shared_ptr<PrimitiveObject>& elmView,
        const std::shared_ptr<PrimitiveObject>& interpShape) {
            FEM_Scaler obj = 0;
            size_t nm_elms = shape->quads.size();
            std::vector<double> objBuffer(nm_elms);

            const auto& cpos = shape->attr<zeno::vec3f>("curPos");

            std::vector<std::vector<Vec3d>> interpPs;
            std::vector<std::vector<Vec3d>> interpWs;
            AssignElmInterpShape(nm_elms,interpShape,interpPs,interpWs);
        
            #pragma omp parallel for 
            for(size_t elm_id = 0;elm_id < nm_elms;++elm_id){
                auto tet = shape->quads[elm_id];
                TetAttributes attrbs;
                AssignElmAttribs(elm_id,shape,elmView,attrbs);
                attrbs.interpPenaltyCoeff = elmView->has_attr("embed_PC") ? elmView->attr<float>("embed_PC")[elm_id] : 0;
                attrbs.interpPs = interpPs[elm_id];
                attrbs.interpWs = interpWs[elm_id];

                std::vector<Vec12d> elm_traj(1);
                for(size_t i = 0;i < 4;++i)
                    elm_traj[0].segment(i*3,3) << cpos[tet[i]][0],cpos[tet[i]][1],cpos[tet[i]][2];

                FEM_Scaler elm_obj = 0;
                _intPtr->EvalElmObj(attrbs,
                    muscle->_forceModel,
                    damp->_dampForce,
                    elm_traj,&objBuffer[elm_id]);
            }

            for(size_t elm_id = 0;elm_id < nm_elms;++elm_id){
                obj += objBuffer[elm_id];
            }

            return obj; 
    }

    FEM_Scaler EvalObjDeriv(const std::shared_ptr<PrimitiveObject>& shape,
        const std::shared_ptr<PrimitiveObject>& elmView,
        const std::shared_ptr<PrimitiveObject>& interpShape,
        VecXd& deriv) {
            FEM_Scaler obj = 0;
            size_t nm_elms = shape->quads.size();
            std::vector<double> objBuffer(nm_elms);
            std::vector<Vec12d> derivBuffer(nm_elms);
            
            const auto& cpos = shape->attr<zeno::vec3f>("curPos");

            std::vector<std::vector<Vec3d>> interpPs;
            std::vector<std::vector<Vec3d>> interpWs;
            AssignElmInterpShape(nm_elms,interpShape,interpPs,interpWs);

            #pragma omp parallel for 
            for(size_t elm_id = 0;elm_id < nm_elms;++elm_id){
                auto tet = shape->quads[elm_id];

                TetAttributes attrbs;
                AssignElmAttribs(elm_id,shape,elmView,attrbs);
                attrbs.interpPenaltyCoeff = elmView->has_attr("embed_PC") ? elmView->attr<float>("embed_PC")[elm_id] : 0;
                attrbs.interpPs = interpPs[elm_id];
                attrbs.interpWs = interpWs[elm_id];

                std::vector<Vec12d> elm_traj(1);
                for(size_t i = 0;i < 4;++i)
                    elm_traj[0].segment(i*3,3) << cpos[tet[i]][0],cpos[tet[i]][1],cpos[tet[i]][2];

                _intPtr->EvalElmObjDeriv(attrbs,
                    muscle->_forceModel,
                    damp->_dampForce,
                    elm_traj,&objBuffer[elm_id],derivBuffer[elm_id]);
            }

            deriv.setZero();
            for(size_t elm_id = 0;elm_id < nm_elms;++elm_id){
                obj += objBuffer[elm_id];
                const auto& elm = shape->quads[elm_id];
                AssembleElmVector(elm,derivBuffer[elm_id],deriv);
            }

            return obj;
    }

    FEM_Scaler EvalObjDerivHessian(const std::shared_ptr<PrimitiveObject>& shape,
        const std::shared_ptr<PrimitiveObject>& elmView,
        const std::shared_ptr<PrimitiveObject>& interpShape,
        VecXd& deriv,VecXd& HValBuffer,bool enforce_spd) {
            FEM_Scaler obj = 0;
            size_t clen = _intPtr->GetCouplingLength();
            size_t nm_elms = shape->quads.size();

            std::vector<double> objBuffer(nm_elms);
            std::vector<Vec12d> derivBuffer(nm_elms);
            std::vector<Mat12x12d> HBuffer(nm_elms);

            const auto& cpos = shape->attr<zeno::vec3f>("curPos");

            std::vector<std::vector<Vec3d>> interpPs;
            std::vector<std::vector<Vec3d>> interpWs;
            AssignElmInterpShape(nm_elms,interpShape,interpPs,interpWs);

            #pragma omp parallel for 
            for(size_t elm_id = 0;elm_id < nm_elms;++elm_id){
                auto tet = shape->quads[elm_id];

                TetAttributes attrbs;
                AssignElmAttribs(elm_id,shape,elmView,attrbs);  
                attrbs.interpPenaltyCoeff = elmView->has_attr("embed_PC") ? elmView->attr<float>("embed_PC")[elm_id] : 0;
                attrbs.interpPs = interpPs[elm_id];
                attrbs.interpWs = interpWs[elm_id];   

                std::vector<Vec12d> elm_traj(1);
                for(size_t i = 0;i < 4;++i)
                    elm_traj[0].segment(i*3,3) << cpos[tet[i]][0],cpos[tet[i]][1],cpos[tet[i]][2];


                _intPtr->EvalElmObjDerivJacobi(attrbs,
                    muscle->_forceModel,
                    damp->_dampForce,
                    elm_traj,
                    &objBuffer[elm_id],derivBuffer[elm_id],HBuffer[elm_id],enforce_spd);

                // _intPtr->EvalElmObjDerivJacobi(attrbs,
                //     muscle->_forceModel,
                //     damp->_dampForce,
                //     elm_traj,
                //     &objBuffer[elm_id],derivBuffer[elm_id],HBuffer[elm_id],false);

                // FEM_Scaler obj_tmp;
                // Vec12d deriv_tmp,deriv_fd;
                // Mat12x12d H_fd;
                // std::vector<Vec12d> elm_traj_copy = elm_traj;
                // for(size_t i = 0;i < 12;++i){
                //     elm_traj_copy[0] = elm_traj[0];
                //     FEM_Scaler step = elm_traj_copy[0][i] * 1e-6;
                //     step = fabs(step) < 1e-6 ? 1e-6 : step;

                //     elm_traj_copy[0][i] += step;
                //     _intPtr->EvalElmObjDeriv(attrbs,
                //         muscle->_forceModel,
                //         damp->_dampForce,
                //         elm_traj_copy,&obj_tmp,deriv_tmp);

                //     deriv_fd[i] = (obj_tmp - objBuffer[elm_id]) / step;
                //     H_fd.col(i) = (deriv_tmp - derivBuffer[elm_id]) / step;
                // }

                // FEM_Scaler D_error = (deriv_fd - derivBuffer[elm_id]).norm() / (deriv_fd.norm() + 1e-3);
                // FEM_Scaler H_error = (H_fd - HBuffer[elm_id]).norm() / H_fd.norm();

                // if((D_error > 1e-3 || H_error > 1e-3) && attrbs.interpPs.size() > 0){
                //     std::cout << "ELM_ID : " << attrbs._elmID << std::endl;
                //     std::cout << "NM_INTERP : " << attrbs.interpPs.size() << std::endl;
                //     std::cout << "EMBED_PC : " << attrbs.interpPenaltyCoeff << std::endl;
                //     std::cout << "D_ERROR : " << D_error << std::endl;
                //     std::cout << "H_ERROR : " << H_error << std::endl;
                //     for(int i = 0;i < 12;++i){
                //         std::cout << "D<" << i << "> :\t" << deriv_fd[i] << "\t" << derivBuffer[elm_id][i] << std::endl;

                //     }

                //     throw std::runtime_error("INT_DH_ERROR");
                // }


            }

            deriv.setZero();
            HValBuffer.setZero();

            // std::cout << "SIZE_OF_H_MATRIX : " << HValBuffer.size() << std::endl;

            // std::cout << "ASSEMBLE_HERE" << std::endl;
            for(size_t elm_id = 0;elm_id < nm_elms;++elm_id){
                auto tet = shape->quads[elm_id];
                obj += objBuffer[elm_id];
                // std::cout << "ASSEMBLE_ELM_VECTOR" << std::endl;
                AssembleElmVector(tet,derivBuffer[elm_id],deriv);
                // std::cout << "ASSEMBLE_ELM_MATRIX" << std::endl;
                AssembleElmMatrixAdd(tet,HBuffer[elm_id],MatHelper::MapHMatrixRef(shape->size(),_connMatrix,HValBuffer.data()));
                // std::cout << "FINISH ASSEMBLING" << std::endl;
            }

            // std::cout << "FINISH ASSEMBLING" << std::endl;
            return obj;
    }

    void RetrieveElmCurrentShape(size_t elm_id,Vec12d& elm_vec,const std::shared_ptr<PrimitiveObject>& shape) const{  
        const auto& elm = shape->quads[elm_id];
        const auto& cpos = shape->attr<zeno::vec3f>("curPos");
        for(size_t i = 0;i < 4;++i)
            elm_vec.segment(i*3,3) << cpos[elm[i]][0],cpos[elm[i]][1],cpos[elm[i]][2];
    } 

    void AssembleElmVector(const zeno::vec4i& elm,const Vec12d& elm_vec,VecXd& global_vec) const{
        for(size_t i = 0;i < 4;++i)
            global_vec.segment(elm[i]*3,3) += elm_vec.segment(i*3,3);
            // shape->verts[elm[i]] += zeno::vec3f(elm_vec[i*3 + 0],elm_vec[i*3 + 1],elm_vec[i*3 + 2]);
    }

    void AssembleElmMatrixAdd(const zeno::vec4i& elm,const Mat12x12d& elm_H,Eigen::Map<SpMat> H) const{
        for(size_t i = 0;i < 4;++i) {
            for(size_t j = 0;j < 4;++j)
                for (size_t r = 0; r < 3; ++r)
                    for (size_t c = 0; c < 3; ++c)
                        H.coeffRef(elm[i] * 3 + r, elm[j] * 3 + c) += elm_H(i * 3 + r, j * 3 + c);
        } 
    }

};

};