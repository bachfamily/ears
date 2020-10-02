/*
 // Copyright (c) 2012-2017 CICM - Universite Paris 8 - Labex Arts H2H.
 // Authors :
 // 2012: Pierre Guillot, Eliott Paris & Julien Colafrancesco.
 // 2012-2015: Pierre Guillot & Eliott Paris.
 // 2015: Pierre Guillot & Eliott Paris & Thomas Le Meur (Light version)
 // 2016-2017: Pierre Guillot.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#pragma once

#include "Hoa_Processor.hpp"
#include "Hoa_Y90RotationMatrices.hpp"

#include <Eigen/Geometry>

using namespace Eigen;

namespace hoa
{
    
    // ================================================================================ //
    // CONVERSIONS //
    // ================================================================================ //

    template <typename T>
    Quaternion<T> EulerToQuaternion(T angle_axis1, T angle_axis2, T angle_axis3, int axis1, int axis2, int axis3)
    {
        Quaternion<T> q = AngleAxisf(angle_axis1, axis1 == 0 ? Vector3f::UnitX() : (axis1 == 1 ? Vector3f::UnitY(): Vector3f::UnitZ())) *
        AngleAxis<T>(angle_axis2, axis2 == 0 ? Vector3f::UnitX() : (axis2 == 1 ? Vector3f::UnitY(): Vector3f::UnitZ())) *
        AngleAxis<T>(angle_axis3, axis3 == 0 ? Vector3f::UnitX() : (axis3 == 1 ? Vector3f::UnitY(): Vector3f::UnitZ()));
        return q;
    }
    
    template <typename T>
    Vector3f QuaternionToEuler(Quaternion<T> q, int axis1, int axis2, int axis3)
    {
        return q.toRotationMatrix().eulerAngles(axis1, axis2, axis3);
    }
    
    template <typename T>
    void YawPitchRollToZYZ(const T yaw, const T pitch, const T roll, T* alpha, T* beta, T* gamma)
    {
        Quaternion<T> q = EulerToQuaternion(yaw, pitch, roll, 2, 1, 0);
        Vector3f euler = QuaternionToEuler(q, 2, 1, 2);
        *alpha = euler[0];
        *beta = euler[1];
        *gamma = euler[2];
    }
    
    
    
    
    // ================================================================================ //
    // ROTATE //
    // ================================================================================ //
    
    //! @brief The rotate class rotates a sound field in the harmonics domain (2d available only).
    //! @details Rotate a sound field by weighting the harmonics depending on the rotation.
    template <Dimension D, typename T>
    class Rotate
    : public ProcessorHarmonics<D, T>
    {
    public:
        
        //! @brief Constructor.
        //! @param order The order (minimum 1).
        Rotate(const size_t order) noexcept;
        
        //! @brief Destructor.
        virtual ~Rotate() noexcept = 0;
        
        //! @brief This method sets the angle of the rotation around the z axis, the yaw value.
        //! @details The yaw is equivalent to a rotation around the z axis,
        //! the yaw value \f$\theta\f$ is in radian and should be between \f$0\f$ and \f$2\pi\f$.
        //! @param yaw The yaw value.
        virtual void setYaw(const T yaw) noexcept;
        
        //! @brief Get the angle of the rotation around the z axis, the yaw value.
        //! @details The method returns the angle of the rotation around the z axis, the yaw value \f$\theta\f$, in radian between \f$0\f$ and \f$2\pi\f$.
        //! @return The yaw value.
        virtual T getYaw() const noexcept;
        
        //! @brief This method performs the rotation.
        //! @details You should use this method for in-place or not-in-place processing and sample by sample.
        //! The inputs array and outputs array contains the spherical harmonics samples
        //! and the minimum size must be the number of harmonics.
        //! If \f$l = 0\f$
        //! \f[Y^{rotated}_{0,0}(\theta) = Y_{0,0}\f]
        //! else
        //! \f[Y^{rotated}_{l,-l}(\theta) = \sin{(\theta l)} \times Y_{l,l} + \cos{(\theta l)} \times Y_{l,-l}\f]
        //! and
        //! \f[Y^{rotated}_{l,l}(\theta) = \cos{(\theta l)} \times Y_{l,l} - \sin{(\theta l)} \times Y_{l,-l}\f]
        //! with \f$\theta\f$ the rotation in radian, \f$l\f$ the degree and \f$m\f$ the order.
        //! @param inputs The input array.
        //! @param outputs The output array.
        virtual void process(const T* inputs, T* outputs) noexcept;
    };
    
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    
    // ================================================================================ //
    // ROTATE 2D //
    // ================================================================================ //
    
    //! @brief 2d specialisation.
    template <typename T>
    class Rotate<Hoa2d, T>
    : public ProcessorHarmonics<Hoa2d, T>
    {
    public:
        
        //! @brief Constructor.
        //! @param order The order (minimum 1).
        Rotate(const size_t order)
        : ProcessorHarmonics<Hoa2d, T>(order)
        {}
        
        //! @brief Destructor.
        ~Rotate() = default;
        
        //! @brief This method sets the angle of the rotation around the z axis.
        //! @details The yaw is equivalent to a rotation around the z axis,
        //! the value is in radian and should be between 0 and 2π.
        //! @param yaw The yaw value.
        inline void setYaw(const T yaw)
        {
            m_yaw     =	yaw;
            m_cosyaw    = std::cos(m_yaw);
            m_sinyaw    = std::sin(m_yaw);
        }
        
        //! @brief Get the angle of the rotation around the z axis, the yaw value.
        //! @details Returns the angle of the rotation around the z axis.
        //! The yaw value is in radian between 0 and 2π.
        //! @return The yaw value.
        inline T getYaw() const noexcept
        {
            return math<T>::wrap_two_pi(m_yaw);
        }
        
        //! @brief This method performs the rotation.
        //! @details You should use this method for in-place or not-in-place processing and sample by sample.
        //! The inputs array and outputs array contains the spherical harmonics samples.
        //! The minimum size must be the number of harmonics.
        //! @param inputs   The input array.
        //! @param outputs  The output array.
        inline void process(const T* inputs, T* outputs) noexcept override
        {
            T cos_x = m_cosyaw;
            T sin_x = m_sinyaw;
            T tcos_x = cos_x;
            
            (*outputs++) = (*inputs++);
            T sig = (*inputs++);
            (*outputs++) = sin_x * (*inputs) + cos_x * sig;
            (*outputs++) = cos_x * (*inputs++) - sin_x * sig;
            for(size_t i = 2; i <= ProcessorHarmonics<Hoa2d, T>::getDecompositionOrder(); i++)
            {
                cos_x = tcos_x * m_cosyaw - sin_x * m_sinyaw;
                sin_x = tcos_x * m_sinyaw + sin_x * m_cosyaw;
                tcos_x = cos_x;
                sig = (*inputs++);
                (*outputs++) = sin_x * (*inputs) + cos_x * sig;
                (*outputs++) = cos_x * (*inputs++) - sin_x * sig;
            }
        }
        
    private:
        
        T m_yaw = 0.;
        T m_cosyaw = 0.;
        T m_sinyaw = 0.;
    };
    
    // ================================================================================ //
    // ROTATE 3D //
    // ================================================================================ //
    
    //! @brief 3d specialisation.
    template <typename T>
    class Rotate<Hoa3d, T>
    : public ProcessorHarmonics<Hoa3d, T>
    {
    public:
        
        //! @brief Constructor.
        //! @param order The order (minimum 1, maximum 21).
        Rotate(const size_t order)
        : ProcessorHarmonics<Hoa3d, T>(order)
        {}
        
        //! @brief Destructor.
        ~Rotate() = default;
        
        //! @brief This method sets the angle of the rotation around the z axis.
        //! @details The yaw is equivalent to a rotation around the z axis,
        //! the value is in radian and should be between 0 and 2π.
        //! @param yaw The yaw value.
        inline void setYaw(const T yaw)
        {
            m_yaw     =    yaw;
            preprocess();
        }

        
        //! @brief This method sets the angle of the rotation around the y axis.
        //! @details The roll is equivalent to a rotation around the y axis,
        //! the value is in radian and should be between 0 and 2π.
        //! @param roll The roll value.
        inline void setRoll(const T roll)
        {
            m_roll     =    roll;
            preprocess();
        }

        //! @brief This method sets the angle of the rotation around the x axis.
        //! @details The pitch is equivalent to a rotation around the x axis,
        //! the value is in radian and should be between 0 and 2π.
        //! @param pitch The pitch value.
        inline void setPitch(const T pitch)
        {
            m_pitch     =    pitch;
            preprocess();
        }

        //! @brief Optimized function to set all angles at once.
        //! @details It is equivalent to setYaw() setPitch() setRoll(), but faster,
        //! as it only computes the preprocessing values once.
        //! @param yaw The yaw value.
        //! @param pitch The pitch value.
        //! @param roll The roll value.
        inline void setYawPitchRoll(const T yaw, const T pitch, const T roll)
        {
            m_yaw     =    yaw;
            m_pitch   =    pitch;
            m_roll    =    roll;
            preprocess();
        }

        //! @brief This method sets the angle of the rotation as a quaternion.
        //! @details It is slower than setting yaw pitch roll, because in the
        //! current implementation it falls back to computing yaw/pitch/roll angles
        //! (this can definitely be optimized!).
        //! @param w The w (real) component of the quaternion.
        //! @param x The i component of the quaternion.
        //! @param y The j component of the quaternion.
        //! @param z The k component of the quaternion.
        inline void setQuaternion(const T w, const T x, const T y, const T z)
        {
            Quaternion<T> q(w, x, y, z);
            Vector3f euler = QuaternionToEuler(q, 2, 1, 0);
            setYawPitchRoll(euler[0], euler[1], euler[2]);
        }
        
        //! @brief Get the angle of the rotation around the z axis, the yaw value.
        //! @details Returns the angle of the rotation around the z axis.
        //! The yaw value is in radian between 0 and 2π.
        //! @return The yaw value.
        inline T getYaw() const noexcept
        {
            return math<T>::wrap_two_pi(m_yaw);
        }

        //! @brief Get the angle of the rotation around the y axis, the roll value.
        //! @details Returns the angle of the rotation around the y axis.
        //! The roll value is in radian between 0 and 2π.
        //! @return The roll value.
        inline T getRoll() const noexcept
        {
            return math<T>::wrap_two_pi(m_roll);
        }

        //! @brief Get the angle of the rotation around the x axis, the pitch value.
        //! @details Returns the angle of the rotation around the x axis.
        //! The pitch value is in radian between 0 and 2π.
        //! @return The pitch value.
        inline T getPitch() const noexcept
        {
            return math<T>::wrap_two_pi(m_pitch);
        }
        
        //! @brief This method performs a 90° rotation around the y axis.
        //! @details This is most often useful only in combination with process_Z in
        //! order to build morecomplex rotations.
        //! The minimum size of the input/outpu arrays must be the number of harmonics.
        //! @param inputs   The input array.
        //! @param outputs  The output array.
        inline void process_Y90(const T* inputs, T* outputs) noexcept
        {
            int order = ProcessorHarmonics<Hoa3d, T>::getDecompositionOrder();
            
            // also see https://ambisonics.iem.at/xchange/fileformat/docs/spherical-harmonics-rotation

            int offset = 0;
            for (int o = 0; o <= order; o++) {
                long num_harm_o = o*2 + 1;
                matmul(inputs + offset, outputs + offset, o);
                offset += num_harm_o;
            }
        }
        
        
        // remapping of Daniel's convention to ACN
        // N is the maximum number of harmonics
        inline int remapIdx(int idx, int N)
        {
            // 1 -> N, 2 -> 1, 3 -> N-1, 4 -> 2, etc.
            return (idx % 2 == 0) ? idx / 2 : N - ((idx - 1) / 2);
        }
        
        //! @brief This method performs the rotation around the z axis.
        //! @details You should use this method for in-place or not-in-place processing and sample by sample.
        //! The inputs array and outputs array contains the spherical harmonics samples.
        //! The minimum size must be the number of harmonics.
        //! @param inputs   The input array.
        //! @param outputs  The output array.
        //! @param yaw  The yaw (to be optimized!).
        inline void process_Z(const T* inputs, T* outputs, T* angle_sines, T* angle_cosines) noexcept
        {
            int order = ProcessorHarmonics<Hoa3d, T>::getDecompositionOrder();

            // also see Jerome Daniel's thesis
            // http://gyronymo.free.fr/audio3D/download_Thesis_PwPt.html#Thesis_download
            // ...but we need index remapping, cause Daniel's thesis does not follow ACN convention
            
            int offset = 0;
            for (int o = 0; o <= order; o++) {
                long num_harm_o = o*2 + 1;

                int idxLast = offset + remapIdx(num_harm_o, num_harm_o) - 1;
                outputs[idxLast] = inputs[idxLast];  // last coefficient always stays the same

                // apply block-wise rotation matrix
                for (int b = 0; b < o; b++) { // iteration on blocks of the z-axis rotation matrix
                    int idxA = offset + remapIdx(num_harm_o - 2*b - 1, num_harm_o) - 1;
                    int idxB = offset + remapIdx(num_harm_o - 2*b - 2, num_harm_o) - 1;
                    // THIS IS JUST A ROUGH IMPLEMENTATION
                    // TO BE MASSIVELY OPTIMIZED: cos() and sin() computation can be pre-computed and sin(mt)=... sin(t)
                    outputs[idxA] = inputs[idxA] * angle_cosines[b] - inputs[idxB] * angle_sines[b];
                    outputs[idxB] = inputs[idxA] * angle_sines[b] + inputs[idxB] * angle_cosines[b];
                }
                
                offset += num_harm_o;
            }
        }
        
        inline void copy(const T* inputs, T* outputs, int num_harmonics)
        {
            memcpy(outputs, inputs, num_harmonics * sizeof(T));
        }
        
        inline void preprocess()
        {
            int order = ProcessorHarmonics<Hoa3d, T>::getDecompositionOrder();
            T angle;
            
            // this one is used only for pure Z-axis rotation to optimize this common case
            angle = m_yaw;
            angle = -angle; // rotational convention...
            for (int o = 0; o < order; o++) {
                m_sins_yaw[o] = std::sin((o+1)*angle);
                m_coss_yaw[o] = std::cos((o+1)*angle);
            }

            if ((getPitch() == 0 && getRoll() == 0))
                return; // optimized version for a simple Z-axis rotation case
            
            // otherwise, let's get into the weeds:
            
            YawPitchRollToZYZ(m_yaw, m_pitch, m_roll, &m_alpha, &m_beta, &m_gamma);

            angle = m_gamma + HOA_PI2;
            angle = -angle; // rotational convention...
            for (int o = 0; o < order; o++) {
                m_sins_gamma_pi2[o] = std::sin((o+1)*angle);
                m_coss_gamma_pi2[o] = std::cos((o+1)*angle);
            }

            angle = m_beta + HOA_PI;
            angle = -angle; // rotational convention...
            for (int o = 0; o < order; o++) {
                m_sins_beta_pi[o] = std::sin((o+1)*angle);
                m_coss_beta_pi[o] = std::cos((o+1)*angle);
            }
            
            angle = m_alpha + HOA_PI2;
            angle = -angle; // rotational convention...
            for (int o = 0; o < order; o++) {
                m_sins_alpha_pi2[o] = std::sin((o+1)*angle);
                m_coss_alpha_pi2[o] = std::cos((o+1)*angle);
            }
        }
        
        //! @brief This method performs the rotation.
        //! @details You should use this method for in-place or not-in-place processing and sample by sample.
        //! The inputs array and outputs array contains the spherical harmonics samples.
        //! The minimum size must be the number of harmonics.
        //! @param inputs   The input array.
        //! @param outputs  The output array.
        inline void process(const T* inputs, T* outputs) noexcept override
        {
            int numharmonics = ProcessorHarmonics<Hoa3d, T>::getNumberOfHarmonics();
            if (getPitch() == 0 && getRoll() == 0 && getYaw() == 0) {
                // Copy
                for (int i = 0; i < numharmonics; i++) {
                    outputs[i] = inputs[i];
                }
            } else if (getPitch() == 0 && getRoll() == 0) {
                // Z-axis rotation only, optimized version for this simple case
                process_Z(inputs, outputs, m_sins_yaw, m_coss_yaw);
            } else {
                // full 3d rotation:
                // performs R(alpha, beta, gamma), with alpha, beta, gamma ZYZ euler angles
                // see https://ambisonics.iem.at/xchange/fileformat/docs/spherical-harmonics-rotation
                // R(alpha, beta, gamma) = Rz(alpha+90°) Ry90 Rz(beta+180°) Ry90 Rz(gamma+90°)
                T* temp = new T[numharmonics];

                process_Z(inputs, outputs, m_sins_gamma_pi2, m_coss_gamma_pi2);
                copy(outputs, temp, numharmonics);
                process_Y90(temp, outputs);
                copy(outputs, temp, numharmonics);
                process_Z(temp, outputs, m_sins_beta_pi, m_coss_beta_pi);
                copy(outputs, temp, numharmonics);
                process_Y90(temp, outputs);
                copy(outputs, temp, numharmonics);
                process_Z(temp, outputs, m_sins_alpha_pi2, m_coss_alpha_pi2);

                delete [] temp;
            }
        }
        
    private:
        
        T m_yaw = 0.;
        T m_cosyaw = 0.; // used in 2d rotations
        T m_sinyaw = 0.; // used in 2d rotations

        T m_pitch = 0.;
        T m_roll = 0.;

        // these are the ZYZ euler angles into which we will convert our yaw/pitch/roll:
        T m_alpha;
        T m_beta;
        T m_gamma;
        
        // These fields are used in 3d rotation and, once the preprocess() function has been called
        // contain the sin((i+1)*angle) and cos((i+1)*angle) for i = 0...order-1
        // and for the 3 angles that actually appear in the rotational decomposition as Z-axis rotations,
        // namely: gamma+pi/2, beta+pi, alpha+pi/2
        T m_sins_yaw[HOA_ROTATION_3D_MAXORDER];
        T m_coss_yaw[HOA_ROTATION_3D_MAXORDER];
        T m_sins_gamma_pi2[HOA_ROTATION_3D_MAXORDER];
        T m_sins_beta_pi[HOA_ROTATION_3D_MAXORDER];
        T m_sins_alpha_pi2[HOA_ROTATION_3D_MAXORDER];
        T m_coss_gamma_pi2[HOA_ROTATION_3D_MAXORDER];
        T m_coss_beta_pi[HOA_ROTATION_3D_MAXORDER];
        T m_coss_alpha_pi2[HOA_ROTATION_3D_MAXORDER];
    };

#endif // DOXYGEN_SHOULD_SKIP_THIS
    
}
