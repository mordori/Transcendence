import { useEffect, useRef } from 'react';
import createEngine from './wasm/client.js';

export default function App() {
    const canvasRef = useRef<HTMLCanvasElement>(null);
    const hasBooted = useRef(false);

    useEffect(() => {
        // Guard against React StrictMode double-execution
        if (hasBooted.current) return;
        hasBooted.current = true;

        async function boot() {
            const canvas = canvasRef.current;
            if (!canvas) return;

            console.log("Downloading assets...");
            const [shaderRes, modelRes, modelRes2, modelRes3, modelRes4, modelRes5] = await Promise.all([
                fetch('/assets/shaders/bsdf.wgsl'),
                fetch('/assets/models/ball.glb'),
                fetch('/assets/models/stadium.glb'),
                fetch('/assets/models/stadium_col.glb'),
                fetch('/assets/models/car.glb'),
                fetch('/assets/models/wheel.glb')
            ]);

            if (!shaderRes.ok || !modelRes.ok) {
                console.error("Failed to fetch assets.");
                return;
            }

            const shaderText = await shaderRes.text();
            const modelBuffer = await modelRes.arrayBuffer();
            const modelBuffer2 = await modelRes2.arrayBuffer();
            const modelBuffer3 = await modelRes3.arrayBuffer();
            const modelBuffer4 = await modelRes4.arrayBuffer();
            const modelBuffer5 = await modelRes5.arrayBuffer();

            console.log("Booting C++ Engine...");
            await createEngine({
                canvas: canvas,
                preRun: [(Module: any) => {
                    // 1. Safely create directories
                    try {
                        Module.FS.mkdir('/shaders');
                        Module.FS.mkdir('/models');
                    } catch (e: any) {
                        if (e.code !== 'EEXIST') {
                            console.warn("FS.mkdir error:", e);
                        }
                    }

                    // 2. Write exactly where C++ will look for them
                    try {
                        Module.FS.writeFile('/shaders/bsdf.wgsl', shaderText);
                        Module.FS.writeFile('/models/ball.glb', new Uint8Array(modelBuffer));
                        Module.FS.writeFile('/models/stadium.glb', new Uint8Array(modelBuffer2));
                        Module.FS.writeFile('/models/stadium_col.glb', new Uint8Array(modelBuffer3));
                        Module.FS.writeFile('/models/car.glb', new Uint8Array(modelBuffer4));
                        Module.FS.writeFile('/models/wheel.glb', new Uint8Array(modelBuffer5));
                        console.log("Assets successfully injected.");
                    } catch (e) {
                        console.error("Failed to write assets:", e);
                    }
                }]
            });

            console.log("Engine running!");
        }

        boot().catch(console.error);
    }, []);

    return (
        <div className="engine-container">
            <canvas ref={canvasRef} id="engine-canvas" />
        </div>
    );
}
