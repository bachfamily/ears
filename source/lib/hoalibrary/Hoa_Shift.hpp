/*
 // Author : Daniele Ghisi
 // This is a failed attempt... for now...
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#pragma once

#include "Hoa_Processor.hpp"
#include "Hoa_Y90RotationMatrices.hpp"
#include "Hoa_Rotate.hpp"

#include <Eigen/Geometry>
#include <boost/math/special_functions/bessel.hpp>

#include <iostream>

#define HOA_SHIFT_FFT_SIZE 512
#define HOA_SOUND_SPEED 343
using namespace Eigen;

namespace hoa
{
    
    // ================================================================================ //
    // SHIFT //
    // ================================================================================ //
    
    //! @brief The rotate class shifts a sound field in the harmonics domain (3d available only).
    //! @details Shifts a sound field by weighting the harmonics depending on the translation.
    template <Dimension D, typename T>
    class Shift
    : public ProcessorHarmonics<D, T>
    {
    public:
        
        //! @brief Constructor.
        //! @param order The order (minimum 1).
        Shift(const size_t order) noexcept;
        
        //! @brief Destructor.
        virtual ~Shift() noexcept = 0;
        
        //! @brief This method sets the shift amount.
        //! @details The amount is defined via cartesian coordinates
        //! @param delta_x The shift in the X direction.
        //! @param delta_y The shift in the Y direction.
        //! @param delta_z The shift in the Z direction.
        virtual void setShiftAmount(const T delta_x, const T delta_y, const T delta_z) noexcept;
        
        //! @brief Get the shift amounts.
        //! @details The method fills the x, y, z components of the shift
        virtual void getShiftAmount(const T *delta_x, const T *delta_y, const T *delta_z) const noexcept;
        
        //! @brief This method performs the shift.
        //! @details You should use this method for in-place or not-in-place processing and sample by sample.
        //! The inputs array and outputs array contains the spherical harmonics samples
        //! and the minimum size must be the number of harmonics.
        //! @param inputs A window of input arrays.
        //! @param outputs A window of output arrays.
        virtual void spectral_process(std::vector<std::vector<std::complex<T>>> &inputs, std::vector<std::vector<std::complex<T>>> &outputs) noexcept;

        // unused
        virtual void process(T *inputs, T *outputs) noexcept;
};
    
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    
    // ================================================================================ //
    // SHIFT 2D //
    // ================================================================================ //
    // TO DO
    /*
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
    */
    
    // ================================================================================ //
    // SHIFT 3D //
    // ================================================================================ //
    
    //! @brief 3d specialisation.
    template <typename T>
    class Shift<Hoa3d, T>
    : public ProcessorHarmonics<Hoa3d, T>
    {
    public:
        
        //! @brief Constructor.
        //! @param order The order (minimum 1, maximum 21).
        Shift(const size_t order, const T _samplerate)
        : ProcessorHarmonics<Hoa3d, T>(order), samplerate(_samplerate)
        {
            rotator = new hoa::Rotate<hoa::Hoa3d, std::complex<T>, T>(order);
            invRotator = new hoa::Rotate<hoa::Hoa3d, std::complex<T>, T>(order);
        }
        
        //! @brief Destructor.
        ~Shift() {
            delete rotator;
            delete invRotator;
        }
        
        inline void setShiftAmount(const T delta_x, const T delta_y, const T delta_z)
        {
            m_delta_x = delta_x;
            m_delta_y = delta_y;
            m_delta_z = delta_z;
            preprocess();
        }

        inline T getShiftAmount(const T *delta_x, const T *delta_y, const T *delta_z) const noexcept
        {
            *delta_x = m_delta_x;
            *delta_y = m_delta_y;
            *delta_z = m_delta_z;
        }


        
        inline long ACN(const int order, const long index)
        {
            if (order < 0 || index > order || index < -order) {
                return -1;
            }
            return order*order + order + index;
        }
        
        inline double spherical_bessel_function(int n, double x)
        {
            return x == 0 ? 1 : sqrt(HOA_PI / (2 * x)) * boost::math::cyl_bessel_j(n+0.5, x);
        }

        void print_Zmat(int k)
        {
            std::cout << "Zmat[" << k << "]" << std::endl;
            std::cout << Zmat[k](0,0) << ", " << Zmat[k](0,1) << ", " << Zmat[k](0,2) << ", " << Zmat[k](0,3) << std::endl;
            std::cout << Zmat[k](1,0) << ", " << Zmat[k](1,1) << ", " << Zmat[k](1,2) << ", " << Zmat[k](1,3) << std::endl;
            std::cout << Zmat[k](2,0) << ", " << Zmat[k](2,1) << ", " << Zmat[k](2,2) << ", " << Zmat[k](2,3) << std::endl;
            std::cout << Zmat[k](3,0) << ", " << Zmat[k](3,1) << ", " << Zmat[k](3,2) << ", " << Zmat[k](3,3) << std::endl;
        }
        
        inline unsigned positive_mod(int value, unsigned m) {
            int mod = value % (int)m;
            if (mod < 0) {
                mod += m;
            }
            return mod;
        }
        
        inline T coeffA(long l, long m)
        {
            if ((l >= 0) && (abs(m) <= l))
                return sqrt((( ((T)l) - abs(m) + 1)*(l + abs(m) + 1))/((2*((T)l) + 1)*(2*l + 3)));
            return 0;
        }

        inline T coeffB(long l, long m)
        {
            T b = 0;
            if ((l >= 0) && (abs(m) <= l))
                b = sqrt(((((T)l) - m - 1)*(l - m))/((2*((T)l) - 1)*(2*l + 1)));
            if (m < 0)
                b = -b;
            return b;
        }
        
        inline void car2pol(T x, T y, T z, T *azimuth, T *elevation, T *distance)
        {
            *azimuth = atan2(y,x);
            *elevation = atan2(z,sqrt(x*x + y*y));
            *distance = sqrt(x*x + y*y + z*z);
        }
        
        inline void preprocess_matrix(Matrix<std::complex<T>, Dynamic, Dynamic> &Zmat, double kd)
        {
            int L = ProcessorHarmonics<Hoa3d, T>::getDecompositionOrder();
            long N = ProcessorHarmonics<Hoa3d, T>::getNumberOfHarmonics();
            long NN = (2*L+1)*(2*L+1);
            
            Zmat.resize(N,NN);
            
            // 0) zeroing out stuff
            inputs_vec.resize(N, 1);
            outputs_vec.resize(N, 1);
            for (long i = 0; i < N; i++)
                for (long j = 0; j < NN; j++)
                    Zmat(i,j) = 0.;
            
            // 1) Find terms T^{0,0}_{0,l'}
            for (long lp = 0; lp <= 2*L; lp++) {
                Zmat(ACN(0,0), ACN(lp,0)) = (lp % 2 == 0 ? 1 : -1) * sqrt(2*lp + 1) * spherical_bessel_function(lp, kd);
            }
            
            // 2)
            for (long l = 1; l <= L; l++) {
                long m = l;
                for (long lp = l; lp <= 2*L-l; lp++) {
                    Zmat(ACN(l,m), ACN(lp,m)) = (-(coeffB(lp+1,m-1) * Zmat(ACN(l-1, m-1), ACN(lp+1, m-1))) +
                                                      (coeffB(lp,-m) * Zmat(ACN(l-1, m-1), ACN(lp-1, m-1)))) / (coeffB(l,-m));
                }
            }
            
            // 3)
            for (long m = 0; m <= L - 1; m++) {
                for (long l = m+1; l <= L; l++) {
                    for (long lp = l; lp <= 2*L - l; lp++) {
                        Zmat(ACN(l,m), ACN(lp,m)) = (-(coeffA(lp,m) * Zmat(ACN(l-1,m),ACN(lp+1,m))) +
                                                          (coeffA(lp-1,m) * Zmat(ACN(l-1,m),ACN(lp-1,m))) +
                                                          (coeffA(l-2,m) != 0 ? coeffA(l-2,m) * Zmat(ACN(l-2,m),ACN(lp,m)) : 0)) /
                        (coeffA(l-1,m));
                    }
                }
            }
            
            // 4)
            for (long l = 1; l <= L; l++) {
                for (long lp = l; lp <= L; lp++) {
                    for (long m = 1; m <= l; m++) {
                        Zmat(ACN(l,-m), ACN(lp,-m)) = Zmat(ACN(l,m), ACN(lp,m));
                    }
                }
            }
            
            // 5)
            for (long lp = 0; lp <= L-1; lp++) {
                for (long l = lp+1; l<=L; l++) {
                    for (long m = -lp; m <= lp; m++) {
                        Zmat(ACN(l,m), ACN(lp,m)) = ((T)(((l+lp) % 2 == 0) ? 1. : -1.)) * Zmat(ACN(lp,m), ACN(l,m));
                    }
                }
            }
            
            Zmat.conservativeResize(N, N);
            
            // finally
            std::complex<T> img1 (0,-1);
            for (long l = 0; l <= L; l++) {
                for (long m = -l; m <= l; m++) {
                    for (long lp = 0; lp <= L; lp++) {
                        for (long mp = -lp; mp <= lp; mp++) {
                            if (Zmat(ACN(l,m), ACN(lp,mp)) != (T)0.) {
                                unsigned mod = positive_mod(l-lp, 4);
                                if (mod == 1) {
                                    Zmat(ACN(l,m), ACN(lp,m)) = Zmat(ACN(l,m), ACN(lp,m)) * img1;
                                } else if (mod == 2) {
                                    Zmat(ACN(l,m), ACN(lp,m)) = Zmat(ACN(l,m), ACN(lp,m)) * img1 * img1;
                                } else if (mod == 3) {
                                    Zmat(ACN(l,m), ACN(lp,m)) = Zmat(ACN(l,m), ACN(lp,m)) * img1 * img1 * img1;
                                }
                            }
                        }
                    }
                }
            }
            
            Zmat.transposeInPlace();
        }
        
        inline void preprocess()
        {
            // setting up rotators
            T az, el, d;
            car2pol(m_delta_y, -m_delta_x, m_delta_z, &az, &el, &d);
            
            if (d == 0) {
                // Easy case: no displacement; essentially nothing to do
                rotator->setYawPitchRoll(0, 0, 0);
                invRotator->setYawPitchRoll(0, 0, 0);
                long N = ProcessorHarmonics<Hoa3d, T>::getNumberOfHarmonics();
                for (int bin = 0; bin < HOA_SHIFT_FFT_SIZE / 2 + 1; bin++) {
                    Zmat[bin].resize(N,N);
                    for (long i = 0; i < N; i++)
                        for (long j = 0; j < N; j++)
                            Zmat[bin](i,j) = (i==j ? 1. : 0.); // identity
                }
                return;
            }
            
            T theta = math<T>::wrap_pm_pi(-HOA_PI2-el);
            if (abs(theta) < 0.000001)
                theta = 0;
            if (abs(az) < 0.000001)
                az = 0;
            rotator->setZYZ(0, theta, -az);
            invRotator->setZYZ(az, -theta, 0);
            
            // precomputing matrices
            for (int bin = 0; bin < HOA_SHIFT_FFT_SIZE / 2 + 1; bin++) {
                double freq = samplerate * ((double)bin) / HOA_SHIFT_FFT_SIZE;
                double k = HOA_2PI * freq / HOA_SOUND_SPEED; // wavenumber
                
                if (bin > HOA_SHIFT_FFT_SIZE/2) { // this ensures that the output solutions will be conjug-symmetric and hence will give a real value
                    Zmat[bin] = Zmat[HOA_SHIFT_FFT_SIZE-bin];
                } else if (k*d == 0) { //} || k*d < (2 * HOA_PI *10/HOA_SOUND_SPEED)*0.001) {
                    long N = ProcessorHarmonics<Hoa3d, T>::getNumberOfHarmonics();
                    Zmat[bin].resize(N,N);
                    for (long i = 0; i < N; i++)
                        for (long j = 0; j < N; j++)
                            Zmat[bin](i,j) = (i==j ? 1. : 0.); // identity
                } else {
                   preprocess_matrix(Zmat[bin], k*d);
/*                    { // test with aliasing, doesn't seem to work any better
                        long N = ProcessorHarmonics<Hoa3d, T>::getNumberOfHarmonics();
                        Zmat[bin].resize(N,N);
                        for (long i = 0; i < N; i++)
                            for (long j = 0; j < N; j++)
                                Zmat[bin](i,j) = 0.; // zero-out
                        for (long c = 0; c < 3; c++) {
                            Matrix<std::complex<T>, Dynamic, Dynamic> temp1, temp2;
                            preprocess_matrix(temp1, (HOA_2PI * (c*samplerate + freq) / HOA_SOUND_SPEED)*d);
                            preprocess_matrix(temp2, (HOA_2PI * (c*samplerate + (samplerate - freq)) / HOA_SOUND_SPEED)*d);
                            Zmat[bin] = Zmat[bin] + temp1 + temp2.conjugate();
                        }
                    } */
                }
//                print_Zmat(bin);
            }
        }
        

        //! @brief This method performs the shift.
        //! @details You should use this method for in-place or not-in-place processing and sample by sample.
        //! The inputs array and outputs array contains the spherical harmonics samples.
        //! The minimum size must be the number of harmonics.
        //! The input array must be in the shape (samples, bins)
        //! @param inputs   The input array.
        //! @param outputs  The output array.
        inline void spectral_process(std::vector<std::vector<std::complex<T>>> &inputs, std::vector<std::vector<std::complex<T>>> &outputs) noexcept
        {
            int N = ProcessorHarmonics<Hoa3d, T>::getNumberOfHarmonics();
            if (m_delta_x == 0 && m_delta_y == 0 && m_delta_z == 0) {
                // Copy
                outputs.clear();
                for (long i = 0; i < N; i++) {
                    outputs.push_back(inputs[i]);
                }

            } else {
                // Copy // TO DO OPTIMIZE, NOT NEEDED
                outputs.clear();
                for (long i = 0; i < N; i++) {
                    outputs.push_back(inputs[i]);
                }
                
                // Processing
                std::complex<T> temp_in[N], temp_out[N];
                for (int bin = 0; bin < HOA_SHIFT_FFT_SIZE / 2 + 1; bin++) {
                    
                    // rotating
                    for (long i = 0; i < N; i++)
                        temp_in[i] = inputs[i][bin];
                    rotator->process(temp_in, temp_out); // WHY DOESN'T THIS ROTATE???
                    for (long i = 0; i < N; i++)
                        inputs_vec[i] = temp_out[i];
                    

                    // translation along Z axis
                    outputs_vec = Zmat[bin] * inputs_vec;

                    
                    // inverse rotation again
                    for (long i = 0; i < N; i++)
                        temp_in[i] = outputs_vec[i];
                    invRotator->process(temp_in, temp_out);
                    for (long i = 0; i < N; i++)
                        outputs[i][bin] = temp_out[i];
                     
                }
                for (int bin = HOA_SHIFT_FFT_SIZE / 2 + 1; bin < HOA_SHIFT_FFT_SIZE; bin++) { // enforcing conjugate-consistency
                    for (long i = 0; i < N; i++)
                        outputs[i][bin] = std::conj(outputs[i][HOA_SHIFT_FFT_SIZE - bin]);
                }

            }
        }
        
    private:
        
        T samplerate;
        
        T m_delta_x = 0.;
        T m_delta_y = 0.;
        T m_delta_z = 0.;
        
        // rotators
        Rotate<Hoa3d, std::complex<T>, T> *rotator;
        Rotate<Hoa3d, std::complex<T>, T> *invRotator;

        // matrices
        Matrix<std::complex<T>, Dynamic, Dynamic> Zmat[HOA_SHIFT_FFT_SIZE/2 + 1];
        Matrix<std::complex<T>, Dynamic, 1> inputs_vec, outputs_vec;
    };

#endif // DOXYGEN_SHOULD_SKIP_THIS
    
}
