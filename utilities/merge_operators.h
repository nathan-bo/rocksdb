#ifndef MERGE_OPERATORS_H
#define MERGE_OPERATORS_H

#include <memory>
#include <stdio.h>

#include "rocksdb/merge_operator.h"

namespace rocksdb {

class MergeOperators {
 public:
  static std::shared_ptr<MergeOperator> CreatePutOperator();
  static std::shared_ptr<MergeOperator> CreateUInt64AddOperator();
  static std::shared_ptr<MergeOperator> CreateStringAppendOperator();
  static std::shared_ptr<MergeOperator> CreateStringAppendTESTOperator();

  // Will return a different merge operator depending on the string.
  // TODO: Hook the "name" up to the actual Name() of the MergeOperators?
  static std::shared_ptr<MergeOperator> CreateFromStringId(
      const std::string& name) {
    if (name == "put") {
      return CreatePutOperator();
    } else if ( name == "uint64add") {
      return CreateUInt64AddOperator();
    } else if (name == "stringappend") {
      return CreateStringAppendOperator();
    } else if (name == "stringappendtest") {
      return CreateStringAppendTESTOperator();
    } else {
      // Empty or unknown, just return nullptr
      return nullptr;
    }
  }

};

} // namespace rocksdb

#endif
