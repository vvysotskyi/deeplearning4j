// automatically generated by the FlatBuffers compiler, do not modify
/*
 *  ******************************************************************************
 *  *
 *  *
 *  * This program and the accompanying materials are made available under the
 *  * terms of the Apache License, Version 2.0 which is available at
 *  * https://www.apache.org/licenses/LICENSE-2.0.
 *  *
 *  *  See the NOTICE file distributed with this work for additional
 *  *  information regarding copyright ownership.
 *  * Unless required by applicable law or agreed to in writing, software
 *  * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *  * License for the specific language governing permissions and limitations
 *  * under the License.
 *  *
 *  * SPDX-License-Identifier: Apache-2.0
 *  *****************************************************************************
 */
package org.nd4j.graph;

import java.nio.*;
import java.lang.*;
import java.nio.ByteOrder;
import java.util.*;
import com.google.flatbuffers.*;

@SuppressWarnings("unused")
public final class FlatResult extends Table {
  public static FlatResult getRootAsFlatResult(ByteBuffer _bb) { return getRootAsFlatResult(_bb, new FlatResult()); }
  public static FlatResult getRootAsFlatResult(ByteBuffer _bb, FlatResult obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__assign(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public void __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; vtable_start = bb_pos - bb.getInt(bb_pos); vtable_size = bb.getShort(vtable_start); }
  public FlatResult __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public long id() { int o = __offset(4); return o != 0 ? bb.getLong(o + bb_pos) : 0L; }
  public FlatVariable variables(int j) { return variables(new FlatVariable(), j); }
  public FlatVariable variables(FlatVariable obj, int j) { int o = __offset(6); return o != 0 ? obj.__assign(__indirect(__vector(o) + j * 4), bb) : null; }
  public int variablesLength() { int o = __offset(6); return o != 0 ? __vector_len(o) : 0; }
  public FlatTiming timing(int j) { return timing(new FlatTiming(), j); }
  public FlatTiming timing(FlatTiming obj, int j) { int o = __offset(8); return o != 0 ? obj.__assign(__indirect(__vector(o) + j * 4), bb) : null; }
  public int timingLength() { int o = __offset(8); return o != 0 ? __vector_len(o) : 0; }
  public long footprintForward() { int o = __offset(10); return o != 0 ? bb.getLong(o + bb_pos) : 0L; }
  public long footprintBackward() { int o = __offset(12); return o != 0 ? bb.getLong(o + bb_pos) : 0L; }

  public static int createFlatResult(FlatBufferBuilder builder,
      long id,
      int variablesOffset,
      int timingOffset,
      long footprintForward,
      long footprintBackward) {
    builder.startObject(5);
    FlatResult.addFootprintBackward(builder, footprintBackward);
    FlatResult.addFootprintForward(builder, footprintForward);
    FlatResult.addId(builder, id);
    FlatResult.addTiming(builder, timingOffset);
    FlatResult.addVariables(builder, variablesOffset);
    return FlatResult.endFlatResult(builder);
  }

  public static void startFlatResult(FlatBufferBuilder builder) { builder.startObject(5); }
  public static void addId(FlatBufferBuilder builder, long id) { builder.addLong(0, id, 0L); }
  public static void addVariables(FlatBufferBuilder builder, int variablesOffset) { builder.addOffset(1, variablesOffset, 0); }
  public static int createVariablesVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startVariablesVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addTiming(FlatBufferBuilder builder, int timingOffset) { builder.addOffset(2, timingOffset, 0); }
  public static int createTimingVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startTimingVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addFootprintForward(FlatBufferBuilder builder, long footprintForward) { builder.addLong(3, footprintForward, 0L); }
  public static void addFootprintBackward(FlatBufferBuilder builder, long footprintBackward) { builder.addLong(4, footprintBackward, 0L); }
  public static int endFlatResult(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
  public static void finishFlatResultBuffer(FlatBufferBuilder builder, int offset) { builder.finish(offset); }
  public static void finishSizePrefixedFlatResultBuffer(FlatBufferBuilder builder, int offset) { builder.finishSizePrefixed(offset); }
}

