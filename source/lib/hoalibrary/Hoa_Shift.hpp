/*
 // Author : Daniele Ghisi
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#pragma once

#include "Hoa_Processor.hpp"
#include "Hoa_Y90RotationMatrices.hpp"

#include <Eigen/Geometry>
#include <boost/math/special_functions/bessel.hpp>

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
        //! @param inputs The input array.
        //! @param outputs The output array.
        virtual void process(const T* inputs, T* outputs) noexcept;
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
        Shift(const size_t order)
        : ProcessorHarmonics<Hoa3d, T>(order)
        {}
        
        //! @brief Destructor.
        Shift() = default;
        
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
        
        inline double spherical_bessel_function(int n, double x)
        {
            return sqrt(3.1415926536 / (2 * x)) * boost::math::cyl_bessel_j(n+0.5, x);
        }
        
        void print_Zmat()
        {
            std::cout << "Zmat: " << std::endl;
            std::cout << Zmat(0,0) << ", " << Zmat(0,1) << ", " << Zmat(0,2) << ", " << Zmat(0,3) << std::endl;
            std::cout << Zmat(1,0) << ", " << Zmat(1,1) << ", " << Zmat(1,2) << ", " << Zmat(1,3) << std::endl;
            std::cout << Zmat(2,0) << ", " << Zmat(2,1) << ", " << Zmat(2,2) << ", " << Zmat(2,3) << std::endl;
            std::cout << Zmat(3,0) << ", " << Zmat(3,1) << ", " << Zmat(3,2) << ", " << Zmat(3,3) << std::endl;
        }
        
        inline void preprocess()
        {
            int L = ProcessorHarmonics<Hoa3d, T>::getDecompositionOrder();
            long N = ProcessorHarmonics<Hoa3d, T>::getNumberOfHarmonics();
            long NN = (2*L+2)*(2*L+2);

            double d = sqrt(((double)m_delta_x) * ((double)m_delta_x) + ((double)m_delta_y) * ((double)m_delta_y) + ((double)m_delta_z) * ((double)m_delta_z));
            
            Zmat.resize(N,NN);
            a.resize(NN,1);
            b.resize(NN,1);

            // 0) zeroing out stuff
            inputs_vec.resize(N, 1);
            outputs_vec.resize(N, 1);
            for (long i = 0; i < N; i++)
                for (long j = 0; j < NN; j++)
                    Zmat(i,j) = 0.;
            for (long i = 0; i < NN; i++) {
                a(i,0) = 0.;
                b(i,0) = 0.;
            }
            
            print_Zmat();
            
            // 0b) getting recurrence coefficients
            for (long l = 0; l <= 2*L+1; l++) { // TODO check if L is enough or 2L is needed
                for (long m = -l; m <= l; m++) {
                    long i = ACN(l,m);
                    a(i,0) = sqrt( ((T)((l - abs(m) + 1) * (l + abs(m) + 1))) / ((2*l+1) * (2*l + 3)) );
                }
            }
            for (long l = 0; l <= 2*L+1; l++) { // TODO check if L is enough or 2L is needed
                for (long m = 0; m <= l; m++) {
                    long i = ACN(l,m);
                    b(i,0) = sqrt( ((T)((l-m-1)*(l-m)))/((2*l-1)*(2*l+1)) );
                }
                for (long m = -l; m < 0; m++) {
                    long i = ACN(l,m);
                    b(i,0) = -sqrt( ((T)((l-m-1)*(l-m)))/((2*l-1)*(2*l+1)) );
                }
            }

            print_Zmat();


            // 1) Find terms T^{0,0}_{0,l'}
            double freq = 880;
            double k = HOA_2PI * freq / 343.; // 1.; // WHY DO WE PUT k HERE???? WHAT DO WE DO?? IS k the wave number? or the speed? or what
            for (long lp = 0; lp <= 2*L; lp++) {
                Zmat(ACN(0,0), ACN(lp,0)) = (lp % 2 == 0 ? 1 : -1) * sqrt(2*lp + 1)*spherical_bessel_function(lp, k*d); // k
            }
            
            print_Zmat();

            // 2)
            for (long l = 1; l <= L; l++) {
                for (long lp = l; lp <= 2*L-l; lp++) {
                    long m = l;
                    Zmat(ACN(l,m), ACN(lp,m)) = (-b(ACN(lp+1,m-1),0) * Zmat(ACN(l-1, m-1), ACN(lp+1, m-1)) +
                                                 b(ACN(lp,-m),0) * Zmat(ACN(l-1, m-1), ACN(lp-1, m-1))) / (b(ACN(l,-m), 0));
                }
            }
            
            print_Zmat();

            
            // 3)
            for (long m = 0; m <= L - 1; m++) {
                for (long l = m+1; l <= L; l++) {
                    for (long lp = l; lp <= 2*L - l; lp++) {
                        Zmat(ACN(l,m), ACN(lp,m)) = (-a(ACN(lp,m),0) * Zmat(ACN(l-1,m),ACN(lp+1,m)) +
                                                     a(ACN(lp-1,m),0) * Zmat(ACN(l-1,m),ACN(lp-1,m)) +
                                                     (abs(m) <= lp-2 && abs(m) <= l-2 ?
                                                      a(ACN(lp-2,m),0) * Zmat(ACN(l-2,m),ACN(lp,m)) : 0));
                    }
                }
            }
            
            print_Zmat();

            
            // 4)
            for (long l = 1; l <= L; l++) {
                for (long lp = l; lp <= L; lp++) {
                    for (long m = 1; m <= l; m++) {
                        Zmat(ACN(l,-m), ACN(lp,-m)) = Zmat(ACN(l,m), ACN(lp,m));
                    }
                }
            }
            
            print_Zmat();

            // 5)
            for (long lp = 0; lp <= L-1; lp++) {
                for (long l = lp+1; l<=L; l++) {
                    for (long m = -lp; m <= lp; m++) {
                        Zmat(ACN(l,m), ACN(lp,m)) = ((T)(((l+lp) % 2 == 0) ? 1. : -1.)) * Zmat(ACN(lp,m), ACN(l,m));
                    }
                }
            }
            
            print_Zmat();
            
            Zmat.conservativeResize(N, N);

            // finally
            std::complex<T> img1 (0,-1);
            for (long l = 0; l <= L; l++) {
                for (long m = -l; m <= l; m++) {
                    for (long lp = 0; lp <= L; lp++) {
                        if (abs(m) <= lp) {
                            if (Zmat(ACN(l,m), ACN(lp,m)) != (T)0.) {
                                if ((lp-l) % 4 == 1) {
                                    Zmat(ACN(l,m), ACN(lp,m)) = Zmat(ACN(l,m), ACN(lp,m)) * img1;
                                } else if ((lp-l) % 4 == 2) {
                                    Zmat(ACN(l,m), ACN(lp,m)) = Zmat(ACN(l,m), ACN(lp,m)) * img1 * img1;
                                } else if ((lp-l) % 4 == 3) {
                                    Zmat(ACN(l,m), ACN(lp,m)) = Zmat(ACN(l,m), ACN(lp,m)) * img1 * img1 * img1;
                                }
                            }
                        }
                    }
                }
            }
        }
        
        //! @brief This method performs the shift.
        //! @details You should use this method for in-place or not-in-place processing and sample by sample.
        //! The inputs array and outputs array contains the spherical harmonics samples.
        //! The minimum size must be the number of harmonics.
        //! @param inputs   The input array.
        //! @param outputs  The output array.
        inline void process(const T* inputs, T* outputs) noexcept override
        {
            int numharmonics = ProcessorHarmonics<Hoa3d, T>::getNumberOfHarmonics();
            if (m_delta_x == 0 && m_delta_y == 0 && m_delta_z == 0) {
                // Copy
                for (int i = 0; i < numharmonics; i++) {
                    outputs[i] = inputs[i];
                }
            } else if (m_delta_x == 0 && m_delta_y == 0) {
                // Z-axis shift only, optimized version for this simple case
                int N = ProcessorHarmonics<Hoa3d, T>::getNumberOfHarmonics();
                for (long i = 0; i < N; i++) {
                    // TO DO, optimize heavily!
                    long order = floor(sqrt(N));
                    T normalization_conversion = sqrt(2*order + 1);
                    inputs_vec(i,0) = inputs[i] * normalization_conversion; // convert to N3D, because I'm following a N3D convention
                }
                
                outputs_vec = Zmat * inputs_vec;

/*                print_Zmat();
                
                std::cout << "Inputs: " << std::endl;
                std::cout << inputs_vec(0,0) << ", " << inputs_vec(1,0) << ", " << inputs_vec(2,0) << ", " << inputs_vec(3,0) << std::endl;

                std::cout << "Outputs: " << std::endl;
                std::cout << outputs_vec(0,0) << ", " << outputs_vec(1,0) << ", " << outputs_vec(2,0) << ", " << outputs_vec(3,0) << std::endl;
  */
                
                for (long i = 0; i < N; i++) {
                    // TO DO, optimize heavily!
                    long order = floor(sqrt(N));
                    T normalization_conversion = sqrt(2*order + 1);
                    outputs[i] = outputs_vec(i,0).real() / normalization_conversion;  // convert back to SN3D, because I'm following a N3D convention
                };
            } else {
                // TO DO
            }
        }
        
    private:
        
        T m_delta_x = 0.;
        T m_delta_y = 0.;
        T m_delta_z = 0.;
        
        // recurrence coefficients
        Matrix<T, Dynamic, 1> a;
        Matrix<T, Dynamic, 1> b;

        // matrices
        Matrix<std::complex<T>, Dynamic, Dynamic> Zmat;
        Matrix<std::complex<T>, Dynamic, 1> inputs_vec, outputs_vec;
    };

#endif // DOXYGEN_SHOULD_SKIP_THIS
    
}
