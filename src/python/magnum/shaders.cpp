/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019
              Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> /* for vector arguments */
#include <Corrade/Containers/ArrayViewStl.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Shaders/VertexColor.h>

#include "corrade/EnumOperators.h"
#include "magnum/bootstrap.h"
#include "magnum/NonDestructible.h"

namespace magnum { namespace {

template<UnsignedInt dimensions> void vertexColor(NonDestructibleBase<Shaders::VertexColor<dimensions>, GL::AbstractShaderProgram>& c) {
    /* Attributes */
    c.attr("COLOR3") = GL::DynamicAttribute{
        GL::DynamicAttribute::Kind::Generic, 3,
        GL::DynamicAttribute::Components::Three,
        GL::DynamicAttribute::DataType::Float};
    c.attr("COLOR4") = GL::DynamicAttribute{
        GL::DynamicAttribute::Kind::Generic, 3,
        GL::DynamicAttribute::Components::Four,
        GL::DynamicAttribute::DataType::Float};

    /* Methods */
    c
        .def(py::init(), "Constructor")

        /* Using lambdas to avoid method chaining getting into signatures */

        // TODO: make writeonly once https://github.com/pybind/pybind11/pull/1144
        // is released
        .def_property("transformation_projection_matrix", [](Shaders::VertexColor<dimensions>&) {
            return MatrixTypeFor<dimensions, Float>{};
        }, &Shaders::VertexColor<dimensions>::setTransformationProjectionMatrix,
            "Transformation and projection matrix");
}

void shaders(py::module& m) {
    m.import("magnum.gl");

    /* 2D/3D vertex color shader */
    {
        NonDestructibleBase<Shaders::VertexColor2D, GL::AbstractShaderProgram> vertexColor2D{m,
            "VertexColor2D", "2D vertex color shader"};
        NonDestructibleBase<Shaders::VertexColor3D, GL::AbstractShaderProgram> vertexColor3D{m,
            "VertexColor3D", "3D vertex color shader"};
        vertexColor2D.attr("POSITION") = GL::DynamicAttribute{
            GL::DynamicAttribute::Kind::Generic, 0,
            GL::DynamicAttribute::Components::Two,
            GL::DynamicAttribute::DataType::Float};
        vertexColor3D.attr("POSITION") = GL::DynamicAttribute{
            GL::DynamicAttribute::Kind::Generic, 0,
            GL::DynamicAttribute::Components::Three,
            GL::DynamicAttribute::DataType::Float};
        vertexColor(vertexColor2D);
        vertexColor(vertexColor3D);
    }

    /* Phong shader */
    {
        NonDestructibleBase<Shaders::Phong, GL::AbstractShaderProgram> phong{m,
            "Phong", "Phong shader"};
        phong.attr("POSITION") = GL::DynamicAttribute{
            GL::DynamicAttribute::Kind::Generic, 0,
            GL::DynamicAttribute::Components::Three,
            GL::DynamicAttribute::DataType::Float};
        phong.attr("TEXTURE_COORDINATES") = GL::DynamicAttribute{
            GL::DynamicAttribute::Kind::Generic, 1,
            GL::DynamicAttribute::Components::Two,
            GL::DynamicAttribute::DataType::Float};
        phong.attr("NORMAL") = GL::DynamicAttribute{
            GL::DynamicAttribute::Kind::Generic, 2,
            GL::DynamicAttribute::Components::Three,
            GL::DynamicAttribute::DataType::Float};

        py::enum_<Shaders::Phong::Flag> flags{phong, "Flags", "Flags"};

        flags
            .value("AMBIENT_TEXTURE", Shaders::Phong::Flag::AmbientTexture)
            .value("DIFFUSE_TEXTURE", Shaders::Phong::Flag::DiffuseTexture)
            .value("SPECULAR_TEXTURE", Shaders::Phong::Flag::SpecularTexture)
            .value("ALPHA_MASK", Shaders::Phong::Flag::AlphaMask)
            .value("NONE", Shaders::Phong::Flag{});
        corrade::enumOperators(flags);

        phong
            .def(py::init<Shaders::Phong::Flag, UnsignedInt>(), "Constructor",
                py::arg("flags") = Shaders::Phong::Flag{},
                py::arg("light_count") = 1)
            .def_property_readonly("flags", [](Shaders::Phong& self) {
                return Shaders::Phong::Flag(UnsignedByte(self.flags()));
            }, "Flags")
            .def_property_readonly("light_count", &Shaders::Phong::lightCount,
                "Light count")
            // TODO: make write-only once https://github.com/pybind/pybind11/pull/1144
            // is released
            .def_property("ambient_color", [](Shaders::Phong&) { return Color4{}; },
                &Shaders::Phong::setAmbientColor, "Ambient color")
            .def_property("diffuse_color", [](Shaders::Phong&) { return Color4{}; },
                &Shaders::Phong::setDiffuseColor, "Diffuse color")
            .def_property("specular_color", [](Shaders::Phong&) { return Color4{}; },
                &Shaders::Phong::setSpecularColor, "Specular color")
            // TODO: textures, once exposed
            .def_property("shininess", [](Shaders::Phong&) { return Float{}; },
                &Shaders::Phong::setShininess, "Shininess")
            .def_property("alpha_mask", [](Shaders::Phong&) { return Float{}; },
                &Shaders::Phong::setAlphaMask, "Alpha mask")
            .def_property("transformation_matrix", [](Shaders::Phong&) { return Matrix4{}; },
                &Shaders::Phong::setTransformationMatrix, "Set transformation matrix")
            .def_property("normal_matrix", [](Shaders::Phong&) { return Matrix3x3{}; },
                &Shaders::Phong::setNormalMatrix, "Set normal matrix")
            .def_property("projection_matrix", [](Shaders::Phong&) { return Matrix4{}; },
                &Shaders::Phong::setProjectionMatrix, "Set projection matrix")
            .def_property("light_positions", [](Shaders::Phong&) {
                return std::vector<Vector3>{};
            }, [](Shaders::Phong& self, const std::vector<Vector3>& positions) {
                self.setLightPositions(positions);
            }, "Light positions")
            .def_property("light_colors", [](Shaders::Phong&) {
                return std::vector<Color4>{};
            }, [](Shaders::Phong& self, const std::vector<Color4>& colors) {
                self.setLightColors(colors);
            }, "Light colors");
    }
}

}}

PYBIND11_MODULE(shaders, m) {
    m.doc() = "Builtin shaders";

    magnum::shaders(m);
}