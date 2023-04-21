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

#include <type_traits>
#ifdef WIN_VERSION
#include <complex>
#endif

namespace hoa
{
    template<typename T>
    struct is_complex_t : public std::false_type {};
    
    template<typename T>
    struct is_complex_t<std::complex<T>> : public std::true_type {};

    
    //! Math utilities and funtions.
    template<typename T>
    class math
    {
    public:
        
        math() = delete;
        ~math() = delete;
        
        //! @brief Returns π constant
        static constexpr T pi() { return 3.14159265358979323846264338327950288; }
        
        //! @brief Returns 2π
        static constexpr T two_pi() { return pi() * (T)2.; }
        
        //! @brief Returns π/2
        static constexpr T pi_over_two() { return pi() * (T)0.5; }
        
        //! @brief Returns π/4
        static constexpr T pi_over_four() { return pi() * (T)0.25; }
        
        //! @brief Wraps the value between 0 and π
        static T wrap_pi(T val)
        {
            while(val < -pi()) { val += two_pi(); }
            while(val >= pi()) { val -= two_pi(); }
            return val;
        }
        
        //! @brief Wraps the value between 0 and 2π
        static T wrap_two_pi(T val)
        {
            while(val < 0.) { val += two_pi(); }
            while(val >= two_pi()) { val -= two_pi(); }
            return val;
        }

        //! @brief Wraps the value between -π and π
        static T wrap_pm_pi(T val)
        {
            while(val < 0.) { val += two_pi(); }
            while(val >= two_pi()) { val -= two_pi(); }
            return val <= pi() ? val : val - two_pi();
        }
        
    };
}
