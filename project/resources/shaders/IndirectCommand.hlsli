struct VertexBufferView
{
    uint2 bufferLocation;
    uint sizeInBytes;
    uint strideInBytes;
};

struct IndexBufferView
{
    uint2 bufferLocation;
    uint sizeInBytes;
    uint format;
};

struct DrawIndexedArguments
{
    uint IndexCountPerInstance;
    uint InstanceCount;
    uint StartIndexLocation;
    int BaseVertexLocation;
    uint StartInstanceLocation;
};
    
struct IndirectCommand
{
    uint instanceOffset;
    VertexBufferView vertexBufferView;
    IndexBufferView indexBufferView;
    DrawIndexedArguments drawIndexedArguments;
};

struct ObjectState
{
    uint visible;
    uint meshIndex;
    uint instanceIndex;
};

struct IndirectCommandState
{
    uint commandIndex;
    uint instanceCount;
    uint currentOffset;
    uint instanceOffset;
};

static const uint kInvalidCommandIndex = (uint) -1;