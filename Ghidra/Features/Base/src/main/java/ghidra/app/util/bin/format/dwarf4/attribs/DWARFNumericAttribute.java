/* ###
 * IP: GHIDRA
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package ghidra.app.util.bin.format.dwarf4.attribs;

import ghidra.program.model.scalar.Scalar;

/**
 * DWARF numeric attribute.
 */
public class DWARFNumericAttribute extends Scalar implements DWARFAttributeValue {

	/**
	 * Creates a new numeric value, using 64 bits and marked as signed
	 * 
	 * @param value long 64 bit value
	 */
	public DWARFNumericAttribute(long value) {
		this(64, value, true);
	}

	/**
	 * Creates a new numeric value, using the specific bitLength and value.
	 * 
	 * @param bitLength number of bits, valid values are 1..64, or 0 if value is also 0
	 * @param value value of the scalar, any bits that are set above bitLength will be ignored
	 * @param signed true for a signed value, false for an unsigned value. 
	 */
	public DWARFNumericAttribute(int bitLength, long value, boolean signed) {
		super(bitLength, value, signed);
	}

	@Override
	public String toString() {
		return String.format("DWARFNumericAttribute: %d [%08x]", getValue(), getValue());
	}
}
