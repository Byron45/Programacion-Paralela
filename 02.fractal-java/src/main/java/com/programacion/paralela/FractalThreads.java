package com.programacion.paralela;

import static com.programacion.paralela.FractalParam.*;

public class FractalThreads {
    private int threadId;
    private int numThreads;
    private double xMin, yMin, xMax, yMax;
    private int width, height;
    private int[] pixelBuffer;


    private FractalThreads(int threadId, int numThreads, double xMin, double yMin,
                           double xMax, double yMax, int width, int height, int[] pixelBuffer) {
        this.threadId = threadId;
        this.numThreads = numThreads;
        this.xMin = xMin;
        this.yMin = yMin;
        this.xMax = xMax;
        this.yMax = yMax;
        this.width = width;
        this.height = height;
        this.pixelBuffer = pixelBuffer;
    }

    public FractalThreads(int dummy, int dummy2, double xMin, double yMin,
                          double xMax, double yMax, int width, int height) {
        this.xMin = xMin;
        this.yMin = yMin;
        this.xMax = xMax;
        this.yMax = yMax;
        this.width = width;
        this.height = height;
        this.numThreads = Runtime.getRuntime().availableProcessors();
    }


    public void julia_parallel() {
        int[] buffer = new int[width * height];
        int numThreads = Runtime.getRuntime().availableProcessors();

        Thread[] threads = new Thread[numThreads];
        WorkerThread[] workers = new WorkerThread[numThreads];

        for (int t = 0; t < numThreads; t++) {
            workers[t] = new WorkerThread(t, numThreads, xMin, yMin, xMax, yMax, width, height, buffer);
            threads[t] = new Thread(workers[t]);
            threads[t].start();
        }

        try {
            for (Thread thread : threads) {
                thread.join();
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        System.arraycopy(buffer, 0, FractalCpu.pixelBuffer, 0, width * height);
    }

    private static class WorkerThread implements Runnable {
        private int threadId;
        private int numThreads;
        private double xMin, yMin, xMax, yMax;
        private int width, height;
        private int[] pixelBuffer;

        WorkerThread(int threadId, int numThreads, double xMin, double yMin,
                     double xMax, double yMax, int width, int height, int[] pixelBuffer) {
            this.threadId = threadId;
            this.numThreads = numThreads;
            this.xMin = xMin;
            this.yMin = yMin;
            this.xMax = xMax;
            this.yMax = yMax;
            this.width = width;
            this.height = height;
            this.pixelBuffer = pixelBuffer;
        }

        @Override
        public void run() {
            double dx = (xMax - xMin) / width;
            double dy = (yMax - yMin) / height;

            for (int i = threadId; i < width; i += numThreads) {
                for (int j = 0; j < height; j++) {
                    double x = xMin + i * dx;
                    double y = yMax - j * dy;

                    var color = FractalCpu.acotado_2(x, y);
                    pixelBuffer[j * width + i] = color;
                }
            }
        }
    }
}
