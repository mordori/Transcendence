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
            const [shaderRes, modelRes, modelRes2, modelRes3, modelRes4, audioRes] = await Promise.all([
                fetch('/assets/shaders/bsdf.wgsl'),
                fetch('/assets/models/ball.glb'),
                fetch('/assets/models/cylinder.glb'),
                fetch('/assets/models/car.glb'),
                fetch('/assets/models/wheel.glb'),
                fetch('/assets/audio/test.wav')
            ]);

            const responses = [shaderRes, modelRes, modelRes2, modelRes3, modelRes4, audioRes];
            const failed = responses.filter(r => !r.ok);
            if (failed.length > 0) {
                console.error("Failed to fetch assets:", failed.map(r => `${r.status} ${r.url}`));
                return;
            }

            const shaderText = await shaderRes.text();
            const modelBuffer = await modelRes.arrayBuffer();
            const modelBuffer2 = await modelRes2.arrayBuffer();
            const modelBuffer3 = await modelRes3.arrayBuffer();
            const modelBuffer4 = await modelRes4.arrayBuffer();
            const audioBuf = await audioRes.arrayBuffer();

            console.log("Booting C++ Engine...");
            await createEngine({
                canvas: canvas,
                preRun: [(Module: any) => {
                    // 1. Safely create directories
                    try {
                        Module.FS.mkdir('/shaders');
                        Module.FS.mkdir('/models');
                        Module.FS.mkdir('/audio');
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
                        Module.FS.writeFile('/models/car.glb', new Uint8Array(modelBuffer3));
                        Module.FS.writeFile('/models/wheel.glb', new Uint8Array(modelBuffer4));
                        Module.FS.writeFile('/audio/test.wav', new Uint8Array(audioBuf));
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
