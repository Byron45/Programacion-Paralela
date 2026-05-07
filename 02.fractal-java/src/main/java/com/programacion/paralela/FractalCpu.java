package com.programacion.paralela;

import static com.programacion.paralela.FractalParam.*;

public class FractalCpu {


    public static int[] pixelBuffer;

    public FractalCpu() {
        this.pixelBuffer = new int[WIDTH * HEIGHT];
    }

    int acotado_2(double x, double y) {

        int iter = 1;
        double zr = x;
        double zi = y;

        while (iter < FractalParam.maxIteraciones && (zr * zr + zi * zi) < 4.0) {
            double dr = zr * zr - zi * zi + cReal;
            double di = 2.0 * zr * zi + cImag;

            zr = dr;
            zi = di;
            iter++;
        }

        if (iter < FractalParam.maxIteraciones) {
            int index = iter % PALETTE_SIZE;
            return colorRamp[index];
        }

        return 0xFF000000;
    }

    void julia_serial_2(double xMin, double yMin, double xMax, double yMax, int width, int height) {
        double dx = (xMax - xMin) / width;
        double dy = (yMax - yMin) / height;
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                double x = xMin + i * dx;
                double y = yMax - j * dy;

                var color = acotado_2(x, y);

                pixelBuffer[j * width + i] = color;

            }
        }
    }
}
