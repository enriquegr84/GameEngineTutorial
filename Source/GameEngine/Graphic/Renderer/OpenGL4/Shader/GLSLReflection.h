// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef GLSLREFLECTION_H
#define GLSLREFLECTION_H

#include "GameEngineStd.h"

#include "Graphic/Renderer/OpenGL4/OpenGL.h"

// Query a program object for all information relevant to manipulating the
// program at run time.

class GRAPHIC_ITEM GLSLReflection
{
public:
    // Construction.  The input is the handle to a program that was
    // successfully created for the active context.
    GLSLReflection(GLuint handle);

    enum  // Named indices for the 'referencedBy' arrays.
    {
        ST_VERTEX,
        ST_GEOMETRY,
        ST_PIXEL,
        ST_COMPUTE,
        ST_TESSCONTROL,
        ST_TESSEVALUATION
    };

    struct Input
    {
        eastl::string name;
        GLint type;
        GLint location;
        GLint arraySize;
        GLint referencedBy[6];
        GLint isPerPatch;
        GLint locationComponent;
    };

    struct Output
    {
        eastl::string name;
        GLint type;
        GLint location;
        GLint arraySize;
        GLint referencedBy[6];
        GLint isPerPatch;
        GLint locationComponent;
        GLint locationIndex;
    };

    struct Uniform
    {
        eastl::string fullName;
        eastl::string name;
        GLint type;
        GLint location;
        GLint arraySize;
        GLint offset;
        GLint blockIndex;
        GLint arrayStride;
        GLint matrixStride;
        GLint isRowMajor;
        GLint atomicCounterBufferIndex;
        GLint referencedBy[6];
    };

    struct DataBlock
    {
        eastl::string name;
        GLint bufferBinding;
        GLint bufferDataSize;
        GLint referencedBy[6];
        eastl::vector<GLint> activeVariables;
    };

    struct AtomicCounterBuffer
    {
        GLint bufferBinding;
        GLint bufferDataSize;
        GLint referencedBy[6];
        eastl::vector<GLint> activeVariables;
    };

    struct SubroutineUniform
    {
        eastl::string name;
        GLint location;
        GLint arraySize;
        eastl::vector<GLint> compatibleSubroutines;
    };

    struct BufferVariable
    {
        eastl::string fullName;
        eastl::string name;
        GLint type;
        GLint arraySize;
        GLint offset;
        GLint blockIndex;
        GLint arrayStride;
        GLint matrixStride;
        GLint isRowMajor;
        GLint topLevelArraySize;
        GLint topLevelArrayStride;
        GLint referencedBy[6];
    };

    struct TransformFeedbackVarying
    {
        eastl::string name;
        GLint type;
        GLint arraySize;
        GLint offset;
        GLint transformFeedbackBufferIndex;
    };

    struct TransformFeedbackBuffer
    {
        GLint bufferBinding;
        GLint transformFeedbackBufferStride;
        eastl::vector<GLint> activeVariables;
    };

    // Member access.
    inline GLuint GetProgramHandle() const;
    inline eastl::vector<Input> const& GetInputs() const;
    inline eastl::vector<Output> const& GetOutputs() const;
    inline eastl::vector<Uniform> const& GetUniforms() const;
    inline eastl::vector<DataBlock> const& GetUniformBlocks() const;
    inline eastl::vector<BufferVariable> const& GetBufferVariables() const;
    inline eastl::vector<DataBlock> const& GetBufferBlocks() const;
    inline eastl::vector<AtomicCounterBuffer> const& GetAtomicCounterBuffers() const;

    // This will not work on an instance based on a visual program.
    // This instance must correspond to a compute shader only program.
    void GetComputeShaderWorkGroupSize(GLint &numXThreads, GLint& numYThreads, GLint& numZThreads) const;

    // Print to a text file for human readability.
    void Print(std::ofstream& output) const;

private:
    void ReflectProgramInputs();
    void ReflectProgramOutputs();
    void ReflectUniforms();
    void ReflectDataBlocks(GLenum programInterface,
        eastl::vector<DataBlock>& blocks);
    void ReflectAtomicCounterBuffers();
    void ReflectSubroutines(GLenum programInterface,
        eastl::vector<eastl::string>& subroutines);
    void ReflectSubroutineUniforms(GLenum programInterface,
        eastl::vector<SubroutineUniform>& subUniforms);
    void ReflectBufferVariables();
    void ReflectTransformFeedbackVaryings();
    void ReflectTransformFeedbackBuffers();

    GLuint mHandle;
    eastl::vector<Input> mInputs;
    eastl::vector<Output> mOutputs;
    eastl::vector<Uniform> mUniforms;
    eastl::vector<DataBlock> mUniformBlocks;
    eastl::vector<DataBlock> mShaderStorageBlocks;
    eastl::vector<AtomicCounterBuffer> mAtomicCounterBuffers;
    eastl::vector<eastl::string> mVertexSubroutines;
    eastl::vector<eastl::string> mGeometrySubroutines;
    eastl::vector<eastl::string> mPixelSubroutines;
    eastl::vector<eastl::string> mComputeSubroutines;
    eastl::vector<eastl::string> mTessControlSubroutines;
    eastl::vector<eastl::string> mTessEvaluationSubroutines;
    eastl::vector<SubroutineUniform> mVertexSubroutineUniforms;
    eastl::vector<SubroutineUniform> mGeometrySubroutineUniforms;
    eastl::vector<SubroutineUniform> mPixelSubroutineUniforms;
    eastl::vector<SubroutineUniform> mComputeSubroutineUniforms;
    eastl::vector<SubroutineUniform> mTessControlSubroutineUniforms;
    eastl::vector<SubroutineUniform> mTessEvaluationSubroutineUniforms;
    eastl::vector<BufferVariable> mBufferVariables;
    eastl::vector<TransformFeedbackVarying> mTransformFeedbackVaryings;
    eastl::vector<TransformFeedbackBuffer> mTransformFeedbackBuffers;

    // Used by Print() method to mape enums to strings.
    struct EnumMap
    {
        GLenum value;
        eastl::string name;
        eastl::string shaderName;
        unsigned rows; // use actual dim for straight vectors
        unsigned cols; // only use for cols in matrices
        unsigned size; // use 0 for opaques
    };
    static const EnumMap msEnumMap[];
    static unsigned GetEnumSize(GLenum value, GLint arraySize, GLint arrayStride, GLint matrixStride, GLint isRowMajor);
    static eastl::string GetEnumName(GLenum value);
    static eastl::string GetEnumShaderName(GLenum value);
    static eastl::string GetReferencedByShaderList(GLint const referencedBy[6]);

private:
    // TODO: This is a workaround for an apparent bug in the Intel
    // HD 4600 OpenGL 4.3.0 (build 10.18.15.4281 and previous).
    // Sometimes a buffer object in a compute shader is reported as
    // unreferenced when in fact it is referenced.  Remove this once
    // the bug is fixed.
    void IntelWorkaround(eastl::string const& name, GLint results[]);
    bool mVendorIsIntel;
    eastl::map<GLenum, int> mShaderTypeMap;
};

inline GLuint GLSLReflection::GetProgramHandle() const
{
    return mHandle;
}

inline eastl::vector<GLSLReflection::Input> const&
GLSLReflection::GetInputs() const
{
    return mInputs;
}

inline eastl::vector<GLSLReflection::Output> const&
GLSLReflection::GetOutputs() const
{
    return mOutputs;
}

inline eastl::vector<GLSLReflection::Uniform> const&
GLSLReflection::GetUniforms() const
{
    return mUniforms;
}

inline eastl::vector<GLSLReflection::DataBlock> const&
GLSLReflection::GetUniformBlocks() const
{
    return mUniformBlocks;
}

inline eastl::vector<GLSLReflection::BufferVariable> const&
GLSLReflection::GetBufferVariables() const
{
    return mBufferVariables;
}

inline eastl::vector<GLSLReflection::DataBlock> const&
GLSLReflection::GetBufferBlocks() const
{
    return mShaderStorageBlocks;
}

inline eastl::vector<GLSLReflection::AtomicCounterBuffer> const&
GLSLReflection::GetAtomicCounterBuffers() const
{
    return mAtomicCounterBuffers;
}

#endif