import asyncio
import json
import threading
import argparse
from dataclasses import dataclass

import numpy as np
import websockets

import matplotlib
matplotlib.use("TkAgg")
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation


# PacketType from your C++ server (WebSocketManager.cc):
# 0 -> Sin, 1 -> FFT
PACKET_SIN = 0
PACKET_FFT = 1


@dataclass
class ClientConfig:
    amplitude: int = 10000
    frequency: int = 50
    sample_rate_hz: int = 512
    fft_rate: int = 2048
    fft_enable: bool = True

    def to_json_str(self) -> str:
        return json.dumps({
            "amplitude": int(self.amplitude),
            "frequency": int(self.frequency),
            "sample_rate_hz": int(self.sample_rate_hz),
            "fft_rate": int(self.fft_rate),
            "fft_enable": bool(self.fft_enable),
        }, ensure_ascii=False)


class QtWsClient:
    def __init__(self, uri: str, cfg: ClientConfig, max_sin_points: int, max_fft_points: int):
        self.uri = uri
        self.cfg = cfg
        self.max_sin_points = max_sin_points
        self.max_fft_points = max_fft_points

        self._lock = threading.Lock()
        self.sin_buffer: list[int] = []
        self.fft_db_buffer: list[float] = []

        self._stop_evt = threading.Event()

    def stop(self):
        self._stop_evt.set()

    def _append_sin(self, arr: np.ndarray):
        if arr.size == 0:
            return
        # store as Python ints
        with self._lock:
            self.sin_buffer.extend(arr.astype(np.int16).tolist())
            if len(self.sin_buffer) > self.max_sin_points * 4:
                # keep some headroom to avoid frequent slicing
                self.sin_buffer = self.sin_buffer[-self.max_sin_points * 2 :]

    def _set_fft_db(self, mag: np.ndarray):
        if mag.size == 0:
            return

        # Your server already sends magnitude (normalized), half-spectrum (N/2+1).
        # Optional: drop DC bin (0 Hz) so it doesn't dominate
        if mag.size > 1:
            mag = mag[1:]

        eps = 1e-12
        mag_db = 20.0 * np.log10(mag + eps)

        with self._lock:
            # keep only latest FFT frame (like your old client)
            if mag_db.size > self.max_fft_points:
                mag_db = mag_db[: self.max_fft_points]
            self.fft_db_buffer = mag_db.astype(np.float32).tolist()

    async def run(self):
        # Simple reconnect loop
        while not self._stop_evt.is_set():
            try:
                async with websockets.connect(self.uri, max_size=None) as ws:
                    print(f"[WS] Connected: {self.uri}")
                    # Send config as TEXT message (your C++ expects textMessageReceived)
                    cfg_json = self.cfg.to_json_str()
                    await ws.send(cfg_json)
                    print(f"[WS] Sent config: {cfg_json}")

                    # Read loop
                    while not self._stop_evt.is_set():
                        msg = await ws.recv()
                        if isinstance(msg, str):
                            # server replies with {"status":"config received"} or {"error":"..."}
                            print(f"[WS] Text: {msg}")
                            continue

                        if not isinstance(msg, (bytes, bytearray)):
                            continue

                        data = bytes(msg)
                        if len(data) < 1:
                            continue

                        pkt_type = data[0]
                        payload = data[1:]

                        if pkt_type == PACKET_SIN:
                            # int16 little-endian
                            arr = np.frombuffer(payload, dtype="<i2")
                            self._append_sin(arr)

                        elif pkt_type == PACKET_FFT:
                            # float32 little-endian
                            mag = np.frombuffer(payload, dtype="<f4")
                            self._set_fft_db(mag)

            except (websockets.ConnectionClosed, OSError) as e:
                print(f"[WS] Disconnected: {e}")
                # small backoff
                await asyncio.sleep(0.5)
            except Exception as e:
                print(f"[WS] Error: {e}")
                await asyncio.sleep(0.5)


def start_ws_thread(client: QtWsClient):
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    try:
        loop.run_until_complete(client.run())
    finally:
        loop.close()


def main():
    parser = argparse.ArgumentParser(description="Python client for Qt6 C++ WebSocket signal server")
    parser.add_argument("--uri", default="ws://localhost:1037", help="WebSocket URI (default: ws://localhost:1037)")
    parser.add_argument("--amplitude", type=int, default=10000)
    parser.add_argument("--frequency", type=int, default=50)
    parser.add_argument("--sample-rate-hz", type=int, default=512)
    parser.add_argument("--fft-rate", type=int, default=2048)
    parser.add_argument("--fft-enable", action="store_true", default=True)
    parser.add_argument("--fft-disable", action="store_true", default=False)
    parser.add_argument("--max-sin", type=int, default=500, help="Max points to show for SIN")
    parser.add_argument("--max-fft", type=int, default=2048, help="Max points to show for FFT(dB)")
    parser.add_argument("--interval-ms", type=int, default=50, help="Plot refresh interval (ms)")
    args = parser.parse_args()

    fft_enable = True
    if args.fft_disable:
        fft_enable = False
    elif args.fft_enable:
        fft_enable = True

    cfg = ClientConfig(
        amplitude=args.amplitude,
        frequency=args.frequency,
        sample_rate_hz=args.sample_rate_hz,
        fft_rate=args.fft_rate,
        fft_enable=fft_enable,
    )

    client = QtWsClient(
        uri=args.uri,
        cfg=cfg,
        max_sin_points=args.max_sin,
        max_fft_points=args.max_fft,
    )

    # Start WS receiver in background thread
    t = threading.Thread(target=start_ws_thread, args=(client,), daemon=True)
    t.start()

    # Plot
    fig = plt.figure(figsize=(10, 8))
    ax1 = fig.add_subplot(2, 1, 1)
    ax2 = fig.add_subplot(2, 1, 2)

    def update(_frame):
        with client._lock:
            sin = np.array(client.sin_buffer[-client.max_sin_points:], dtype=np.int16) if client.sin_buffer else np.array([], dtype=np.int16)
            fft = np.array(client.fft_db_buffer[:client.max_fft_points], dtype=np.float32) if client.fft_db_buffer else np.array([], dtype=np.float32)

        ax1.cla()
        ax2.cla()

        # SIN
        if sin.size > 0:
            x = np.arange(sin.size)
            ax1.plot(x, sin, linewidth=1.5)
            ax1.set_ylim(-32768, 32767)
        ax1.set_title("Live SIN (int16) from Qt C++ Server")
        ax1.set_xlabel("Samples")
        ax1.set_ylabel("Amplitude")
        ax1.grid(True, alpha=0.3)

        # FFT (dB)
        if fft.size > 0:
            x2 = np.arange(fft.size)
            ax2.plot(x2, fft, linewidth=1.5)
        ax2.set_title("Live FFT (dB) from Qt C++ Server")
        ax2.set_xlabel("Bins (without DC)")
        ax2.set_ylabel("dB (20*log10)")
        ax2.grid(True, alpha=0.3)

        plt.tight_layout()

    ani = FuncAnimation(fig, update, interval=args.interval_ms, cache_frame_data=False)

    try:
        plt.show()
    finally:
        client.stop()


if __name__ == "__main__":
    main()
