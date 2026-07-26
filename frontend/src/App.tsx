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
            const [shaderRes, modelRes, modelRes2] = await Promise.all([
                fetch('/assets/shaders/bsdf.wgsl'),
                fetch('/assets/models/ball.glb'),
                fetch('/assets/models/cylinder.glb')
            ]);

            if (!shaderRes.ok || !modelRes.ok) {
                console.error("Failed to fetch assets.");
                return;
            }

            const shaderText = await shaderRes.text();
            const modelBuffer = await modelRes.arrayBuffer();
            const modelBuffer2 = await modelRes2.arrayBuffer();

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
                        Module.FS.writeFile('/models/cylinder.glb', new Uint8Array(modelBuffer2));
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
