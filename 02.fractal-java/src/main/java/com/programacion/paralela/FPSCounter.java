package com.programacion.paralela;

public class FPSCounter {
    private int fps;
    private int frames;
    private long lastTime;

    public FPSCounter(int fps, int frames, long lastTime) {
        this.fps = fps;
        this.frames = frames;
        this.lastTime = lastTime;
    }

    public FPSCounter() {
        lastTime = System.currentTimeMillis();
        fps = 0;
        frames = 0;
    }

    public int update() {
        frames++;
        long now = System.currentTimeMillis();
        if (now - lastTime > 1000) {
            fps = frames;
            frames = 0;
            lastTime = now;
            System.out.println("***FPS: " + fps + "***");
        }
        return fps;
    }
}
