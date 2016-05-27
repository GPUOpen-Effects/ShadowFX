
//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//


/**
* @file      free_camera.hpp
* @brief     simple camera
*/

#ifndef GU_FREE_CAMERA_HPP
#define GU_FREE_CAMERA_HPP

// #include <glm/gtc/matrix_transform.hpp>
#include <tml/mat.hpp>
#include <cassert>

namespace gu
{

    /**
    * @brief free camera
	* @note right handed coordinate system
    */
    class free_camera
    {
        tml::vec3 vx; // camera x axis (right vector)
        tml::vec3 vy; // camera y axis (up vector)
        tml::vec3 vz; // camera z axis (look at vector)
        tml::vec3 o; // camera position
    public:

        free_camera(free_camera const&) = default;
        free_camera& operator = (free_camera const&) = default;

        /**
        * @brief make a free camera
        * @param pos camera position
        * @param look_at point looking at
        * @param up camera up vector
        */
        explicit free_camera(tml::vec3 const& pos = tml::vec3(0.f, 0.f, 1.f), tml::vec3 const& look_at = tml::vec3(0.f), tml::vec3 const& up = tml::vec3(0.f, 1.f, 0.f))
            : o(pos), vz(-tml::normalize(look_at - pos)), vy(up)
        {
            vx = tml::cross(vy, vz);
            vy = tml::cross(vz, vx);
        }

        /**
        * @brief rotate around the up vector
        * @param a angle of rotation in radian
        */
        void update_yaw(float a)
        {
            vz = tml::vec3(tml::rotate(tml::mat4{}, -a, vy) * tml::vec4(vz, 0.f) );
            vx = tml::cross(vy, vz);
        }

        /**
        * @brief rotate around (0,1,0) vector
        * @param a angle of rotation in radian
        */
        void update_yaw010(float a)
        {
            vy = tml::vec3(0.f, 1.f, 0.f);
            auto rot = tml::rotate(tml::mat4{}, -a, vy);
            vx = tml::vec3(rot * tml::vec4(vx, 0.f));
            vz = tml::vec3(rot * tml::vec4(vz, 0.f));
            vy = cross(vz, vx);
        }

        /**
        * @brief rotate around the right vector
        * @param a angle of rotation in radian
        */
        void update_pitch(float a)
        {
            
            vz = tml::vec3(tml::rotate(tml::mat4{}, a, vx) * tml::vec4(vz, 0.f));
            vy = cross(vz, vx);
        }

        /**
        * @brief move forward or backward
        * @param s step; the sign determines the direction
        */
        void move(float s)
        {
            o = o - s * vz;
        }

        /**
        * @brief move left or right
        * @param s step; the sign determines the direction
        */
        void strafe(float s)
        {
            o = o + s * vx;
        }

        /**
        * @brief obtain the camera view matrix
        * @return view matrix
        */
        tml::mat4 get_view_matrix() const
        {
            return tml::mat4
            {
                vx.x, vy.x, vz.x, 0.f,
                vx.y, vy.y, vz.y, 0.f,
                vx.z, vy.z, vz.z, 0.f,
                -dot(vx, o), -dot(vy, o), -dot(vz, o), 1.f
            };
        }
    };


    /**
    * @brief helper function to get a point light camera given a cube map face
    * @param cube_face cube map face
    * @return point light camera for the given cube face
    */
    inline free_camera get_point_light_camera(tml::vec3 const& light_position, size_t cube_face)
    {
        assert(cube_face < 6);
        tml::vec3 view_dir{};
        tml::vec3 up_dir{};
        switch (cube_face)
        {
        case 0: // -Z
            view_dir = tml::vec3(0, 0, -1);
            up_dir = tml::vec3(0, 1, 0);
            break;
        case 1: // +Z
            view_dir = tml::vec3(0, 0, 1);
            up_dir = tml::vec3(0, 1, 0);
            break;
        case 2: // -X
            view_dir = tml::vec3(-1, 0, 0);
            up_dir = tml::vec3(0, 1, 0);
            break;
        case 3: // +X
            view_dir = tml::vec3(1, 0, 0);
            up_dir = tml::vec3(0, 1, 0);
            break;
        case 4: // -Y
            view_dir = tml::vec3(0, -1, 0);
            up_dir = tml::vec3(0, 0, -1);
            break;
        case 5: // +Y
            view_dir = tml::vec3(0, 1, 0);
            up_dir = tml::vec3(0, 0, 1);
            break;
        default:
            break;
        }
        tml::vec3 look_at = light_position + view_dir;
        return free_camera{ light_position, look_at, up_dir };
    }

} // namespace

#endif // GU_FREE_CAMERA_HPP

