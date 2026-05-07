package com.programacion.paralela;

import java.nio.ByteBuffer;

public class FractalSimd {

    ByteBuffer pixelBuffer;

    public FractalSimd() {
        this.pixelBuffer = ByteBuffer.allocateDirect(FractalParam.WIDTH * FractalParam.HEIGHT * 4);
    }

    public void juliaSimd() {
        FractalDll.INSTANCE.julia_simd(
                FractalParam.xMin, FractalParam.yMin,
                FractalParam.xMax, FractalParam.yMax,
                FractalParam.WIDTH, FractalParam.HEIGHT,
                FractalParam.maxIteraciones, pixelBuffer
        );
    }
}
