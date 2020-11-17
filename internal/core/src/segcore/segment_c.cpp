#include <cstring>

#include "SegmentBase.h"
#include "Collection.h"
#include "segment_c.h"
#include <knowhere/index/vector_index/VecIndex.h>
#include <knowhere/index/vector_index/adapter/VectorAdapter.h>
#include <knowhere/index/vector_index/VecIndexFactory.h>

CSegmentBase
NewSegment(CCollection collection, unsigned long segment_id) {
    auto col = (milvus::segcore::Collection*)collection;

    auto segment = milvus::segcore::CreateSegment(col->get_schema());

    // TODO: delete print
    std::cout << "create segment " << segment_id << std::endl;
    return (void*)segment.release();
}

void
DeleteSegment(CSegmentBase segment) {
    auto s = (milvus::segcore::SegmentBase*)segment;

    // TODO: delete print
    std::cout << "delete segment " << std::endl;
    delete s;
}

//////////////////////////////////////////////////////////////////

int
Insert(CSegmentBase c_segment,
       long int reserved_offset,
       signed long int size,
       const long* row_ids,
       const unsigned long* timestamps,
       void* raw_data,
       int sizeof_per_row,
       signed long int count) {
    auto segment = (milvus::segcore::SegmentBase*)c_segment;
    milvus::segcore::RowBasedRawData dataChunk{};

    dataChunk.raw_data = raw_data;
    dataChunk.sizeof_per_row = sizeof_per_row;
    dataChunk.count = count;

    auto res = segment->Insert(reserved_offset, size, row_ids, timestamps, dataChunk);

    // TODO: delete print
    // std::cout << "do segment insert, sizeof_per_row = " << sizeof_per_row << std::endl;
    return res.code();
}

long int
PreInsert(CSegmentBase c_segment, long int size) {
    auto segment = (milvus::segcore::SegmentBase*)c_segment;

    // TODO: delete print
    // std::cout << "PreInsert segment " << std::endl;
    return segment->PreInsert(size);
}

int
Delete(
    CSegmentBase c_segment, long int reserved_offset, long size, const long* row_ids, const unsigned long* timestamps) {
    auto segment = (milvus::segcore::SegmentBase*)c_segment;

    auto res = segment->Delete(reserved_offset, size, row_ids, timestamps);
    return res.code();
}

long int
PreDelete(CSegmentBase c_segment, long int size) {
    auto segment = (milvus::segcore::SegmentBase*)c_segment;

    // TODO: delete print
    // std::cout << "PreDelete segment " << std::endl;
    return segment->PreDelete(size);
}

int
Search(CSegmentBase c_segment,
       CPlan c_plan,
       CPlaceholderGroup* c_placeholder_groups,
       unsigned long* timestamps,
       int num_groups,
       long int* result_ids,
       float* result_distances) {
    auto segment = (milvus::segcore::SegmentBase*)c_segment;
    auto plan = (milvus::query::Plan*)c_plan;
    std::vector<const milvus::query::PlaceholderGroup*> placeholder_groups;
    for (int i = 0; i < num_groups; ++i) {
        placeholder_groups.push_back((const milvus::query::PlaceholderGroup*)c_placeholder_groups[i]);
    }
    milvus::segcore::QueryResult query_result;

    auto res = segment->Search(plan, placeholder_groups.data(), timestamps, num_groups, query_result);

    // result_ids and result_distances have been allocated memory in goLang,
    // so we don't need to malloc here.
    memcpy(result_ids, query_result.result_ids_.data(), query_result.get_row_count() * sizeof(long int));
    memcpy(result_distances, query_result.result_distances_.data(), query_result.get_row_count() * sizeof(float));

    return res.code();
}

//////////////////////////////////////////////////////////////////

int
Close(CSegmentBase c_segment) {
    auto segment = (milvus::segcore::SegmentBase*)c_segment;
    auto status = segment->Close();
    return status.code();
}

int
BuildIndex(CCollection c_collection, CSegmentBase c_segment) {
    auto collection = (milvus::segcore::Collection*)c_collection;
    auto segment = (milvus::segcore::SegmentBase*)c_segment;

    auto status = segment->BuildIndex(collection->get_index());
    return status.code();
}

bool
IsOpened(CSegmentBase c_segment) {
    auto segment = (milvus::segcore::SegmentBase*)c_segment;
    auto status = segment->get_state();
    return status == milvus::segcore::SegmentBase::SegmentState::Open;
}

long int
GetMemoryUsageInBytes(CSegmentBase c_segment) {
    auto segment = (milvus::segcore::SegmentBase*)c_segment;
    auto mem_size = segment->GetMemoryUsageInBytes();
    return mem_size;
}

//////////////////////////////////////////////////////////////////

long int
GetRowCount(CSegmentBase c_segment) {
    auto segment = (milvus::segcore::SegmentBase*)c_segment;
    auto row_count = segment->get_row_count();
    return row_count;
}

long int
GetDeletedCount(CSegmentBase c_segment) {
    auto segment = (milvus::segcore::SegmentBase*)c_segment;
    auto deleted_count = segment->get_deleted_count();
    return deleted_count;
}