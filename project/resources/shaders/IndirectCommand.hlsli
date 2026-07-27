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
    uint meshOffset;
    VertexBufferView vertexBufferView;
    IndexBufferView indexBufferView;
    DrawIndexedArguments drawIndexedArguments;
};

struct MeshCommandState
{
    uint commandIndex;
    uint instanceCount;
    uint currentOffset;
    uint startInstanceLocation;
};

struct MeshLODState
{
    uint meshLODIndex;
    uint visible;
    uint instanceIndex;
};

static const uint kInvalidCommandIndex = (uint) -1;