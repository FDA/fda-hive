/*
 *  ::718604!
 * 
 * Copyright(C) November 20, 2014 U.S. Food and Drug Administration
 * Authors: Dr. Vahan Simonyan (1), Dr. Raja Mazumder (2), et al
 * Affiliation: Food and Drug Administration (1), George Washington University (2)
 * 
 * All rights Reserved.
 * 
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
package owner.isa.hive.utils;

import gov.isa.model.BiologicalMaterial;
import gov.isa.model.GeographicPosition;

public class BioMaterial {
    private String materialName = "";
    private BiologicalMaterial.Predefined materialClass = BiologicalMaterial.Predefined.UNKNOWN;
    private String sampleID = "";
    private boolean isHarmful = false;
    private double confidence = 1.0;
    private String hiveObjectID = "";
    private String hiveSampleID = "";
    private GeographicPosition geoPosition = null;

    public BioMaterial(final String materialName,
                       final BiologicalMaterial.Predefined materialClass,
                       final String sampleID,
                       final boolean isHarmful,
                       final double confidence,
                       final String hiveObjectID,
                       final String hiveSampleID,
                       final GeographicPosition geoPosition) {
        this.materialName = materialName;
        this.materialClass = materialClass;
        this.sampleID = sampleID;
        this.isHarmful = isHarmful;
        this.confidence = confidence;
        this.hiveObjectID = hiveObjectID;
        this.hiveSampleID = hiveSampleID;
        this.geoPosition = geoPosition;
    }

    public final String getMaterialName() {
        return materialName;
    }

    public final BiologicalMaterial.Predefined getMaterialClass() {
        return materialClass;
    }

    public final boolean isHarmful() {
        return isHarmful;
    }

    public final String getSampleID() {
        return sampleID;
    }

    public final double getConfidence() { return confidence; }

    public final String getHiveObjectID() { return hiveObjectID; }

    public final String getHiveSampleID() { return hiveSampleID; }

    public final GeographicPosition getGeoPosition() { return geoPosition; }
}
