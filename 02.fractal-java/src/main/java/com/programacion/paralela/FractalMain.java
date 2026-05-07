package com.programacion.paralela;

import org.lwjgl.*;
import org.lwjgl.glfw.*;
import org.lwjgl.opengl.*;

import java.nio.IntBuffer;

import static org.lwjgl.glfw.Callbacks.*;
import static org.lwjgl.glfw.GLFW.*;
import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.system.MemoryUtil.*;

import static com.programacion.paralela.FractalParam.*;

public class FractalMain {
    //  window handle
    private int textureID;
    private long window;

    private IntBuffer pixelBuffer;

    FractalCpu fractalCpu;
    FractalSimd fractalSimd;
    FPSCounter fpsCounter;

    int modo = 1; //1=CPU, 2=SIMD

    public FractalMain() {
        fractalCpu = new FractalCpu();
        fractalSimd = new FractalSimd();
        fpsCounter = new FPSCounter();

        pixelBuffer = BufferUtils.createIntBuffer(WIDTH * HEIGHT);
    }


    public void run() {
        System.out.println("Fractal Julia " + Version.getVersion() + "!");

        init();
        loop();

        // Free the window callbacks and destroy the window
        glfwFreeCallbacks(window);
        glfwDestroyWindow(window);

        // Terminate GLFW and free the error callback
        glfwTerminate();
        glfwSetErrorCallback(null).free();
    }

    private void init() {
        GLFWErrorCallback.createPrint(System.err).set();

        if (!glfwInit())
            throw new IllegalStateException("Unable to initialize GLFW");

        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        // Create the window
        window = glfwCreateWindow(WIDTH, HEIGHT, "JULIA SET", NULL, NULL);

        if (window == NULL)
            throw new RuntimeException("Failed to create the GLFW window");

        glfwSetKeyCallback(window, (window, key, scancode, action, mods) -> {
            if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
                glfwSetWindowShouldClose(window, true);
            if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
                maxIteraciones += 10;
            }
            if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
                maxIteraciones -= 10;
                if (maxIteraciones < 0) maxIteraciones = 10;
            }

            if (key == GLFW_KEY_1 && action == GLFW_RELEASE) {
                System.out.println("Modo Java CPU");
                modo = 1;
            }
            if (key == GLFW_KEY_2 && action == GLFW_RELEASE) {
                System.out.println("Modo C/C++ SIMD");
                modo = 2;
            }
        });

        GLFWVidMode vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowPos(window,
                (vidmode.width() - WIDTH) / 2,
                (vidmode.height() - HEIGHT) / 2);


        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        glfwShowWindow(window);

        GL.createCapabilities();
        GL.createCapabilitiesWGL();

        String version = GL11.glGetString(GL_VERSION);
        String vendor = GL11.glGetString(GL_VENDOR);
        String renderer = GL11.glGetString(GL_RENDERER);

        System.out.println("Version: " + version);
        System.out.println("Vendor: " + vendor);
        System.out.println("Renderer: " + renderer);

        GL11.glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1, 1, -1, 1, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_TEXTURE_2D);
        glLoadIdentity();

        setupTexture();
    }

    private void setupTexture() {
        textureID = glGenTextures();
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA8,
                WIDTH, HEIGHT, 0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                NULL
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    }

    private void loop() {

        GL.createCapabilities();
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        while (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            paint();
            System.out.println(fpsCounter.update());

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    private void paint() {
        int fps = fpsCounter.update();
        System.out.println("FPS: " + fps);

        pixelBuffer.clear();

        if (modo == 1) {
            fractalCpu.julia_serial_2(xMin, yMin, xMax, yMax, WIDTH, HEIGHT);
            pixelBuffer.put(fractalCpu.pixelBuffer);
        } else if (modo == 2) {
            fractalSimd.juliaSimd();
            pixelBuffer.put(fractalSimd.pixelBuffer.asIntBuffer());
        }

        pixelBuffer.flip(); //resetear el indice en 0

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA8,
                WIDTH, HEIGHT, 0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                pixelBuffer
        );

        glBegin(GL_QUADS);
        {
            glTexCoord2d(0, 0);
            glVertex2d(-1, 1);

            glTexCoord2d(0, 1);
            glVertex2d(-1, -1);

            glTexCoord2d(1, 1);
            glVertex2d(1, -1);

            glTexCoord2d(1, 0);
            glVertex2d(1, 1);

        }

        glEnd();
    }


    public static void main(String[] args) {
        new FractalMain().run();
    }

}

