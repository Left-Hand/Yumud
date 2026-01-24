import asyncio
import serial_asyncio
import tkinter as tk
import math
from typing import List, Optional, Tuple
import threading
from concurrent.futures import ThreadPoolExecutor

DEFAULT_COM_NAME: str = "COM19"
DEFAULT_COM_BAUD: int = 6000000


class PolarPlotter:
    """
    A class to display points in polar coordinates on a canvas.
    """
    def __init__(self, canvas: tk.Canvas, width: int, height: int) -> None:
        self.canvas: tk.Canvas = canvas
        self.width: int = width
        self.height: int = height
        self.center_x: float = width / 2
        self.center_y: float = height / 2
        self.max_radius: float = min(width, height) * 0.4
        
        # Draw polar grid
        self._draw_polar_grid()
        
    def _draw_polar_grid(self) -> None:
        """Draw concentric circles and radial lines for the polar grid."""
        # Draw concentric circles
        for r in range(1, 5):
            radius = self.max_radius * r / 4
            self.canvas.create_oval(
                self.center_x - radius, 
                self.center_y - radius,
                self.center_x + radius, 
                self.center_y + radius,
                outline='lightgray'
            )
            
        # Draw radial lines
        for angle in range(0, 360, 30):
            rad: float = math.radians(angle)
            x: float = self.center_x + self.max_radius * math.cos(rad)
            y: float = self.center_y + self.max_radius * math.sin(rad)
            self.canvas.create_line(self.center_x, self.center_y, x, y, fill='lightgray')
    
    def clear_points(self) -> None:
        """Clear all plotted points from the canvas."""
        self.canvas.delete("point")
        
    def plot_point_xy(self, x: float, y: float, color: str = "red") -> None:
        """
        Plot a point in polar coordinates.
        
        Args:
            x: X coordinate
            y: Y coordinate
            color: Color of the point
        """
        x = x*50 + self.center_x
        y = -y*50 + self.center_y
        # Draw point
        self.canvas.create_oval(
            x - 7, y - 7, x + 7, y + 7,
            fill=color, tags="point"
        )


class AsyncSerialProtocol(asyncio.Protocol):
    """
    Async protocol for handling serial communication
    """
    def __init__(self, callback):
        super().__init__()
        self.callback = callback
        self.buffer = ""
        
    def connection_made(self, transport):
        self.transport = transport
        
    def data_received(self, data):
        # Handle incoming data
        try:
            decoded_data = data.decode('utf-8', errors='ignore')
            self.buffer += decoded_data
            
            # Process complete lines
            while '\n' in self.buffer:
                line, self.buffer = self.buffer.split('\n', 1)
                self.callback(line.strip())
                
        except Exception as e:
            print(f"Error processing received data: {e}")
            
    def connection_lost(self, exc):
        print("Serial connection lost")


class AsyncSerialProcessor:
    """
    Async processor for serial data and updating polar plot.
    """
    def __init__(self, plotter: PolarPlotter) -> None:
        self.plotter: PolarPlotter = plotter
        self.running: bool = False
        self.protocol: Optional[AsyncSerialProtocol] = None
        self.transport = None
        self.loop = None
        
    async def connect(self, port: str, baudrate: int) -> bool:
        """Establish async serial connection."""
        try:
            self.loop = asyncio.get_event_loop()
            self.protocol = AsyncSerialProtocol(self._process_line)
            self.transport, _ = await serial_asyncio.create_serial_connection(
                self.loop, 
                lambda: self.protocol, 
                port, 
                baudrate=baudrate
            )
            return True
        except Exception as e:
            print(f"Failed to connect to {port}: {e}")
            return False
            
    async def disconnect(self) -> None:
        """Close async serial connection."""
        self.running = False
        if self.transport:
            self.transport.close()
            
    def start_reading(self) -> None:
        """Mark as running - actual reading is handled by async protocol"""
        self.running = True
        
    def _process_line(self, line: str) -> None:
        """Process a single line of data from the protocol."""
        if not line:
            return
            
        try:
            # Split by comma and convert to floats
            values: List[str] = line.split(',')
            float_values: List[float] = [float(val.strip()) for val in values]
            # We need at least 2 values for (r, theta)
            if len(float_values) >= 11:
                # Update plot in main thread
                self.plotter.clear_points()
                magnitudes = float_values
                for i in range(0, len(magnitudes)):
                    radians = 2 * math.pi * i / len(magnitudes)
                    self.plotter.plot_point_xy(
                        -magnitudes[i] * math.cos(radians), 
                        -magnitudes[i] * math.sin(radians)
                    )
                
            # print(len(float_values))
                
        except ValueError as e:
            print(f"Error parsing line '{line}': {e}")


class AsyncSerialPolarApp:
    """
    Main application class for the async serial polar plotter.
    """
    def __init__(self, root: tk.Tk) -> None:
        self.root: tk.Tk = root
        self.root.title("Async Serial Polar Plotter")
        self.root.geometry("800x600")
        
        # Initialize event loop
        self.loop = asyncio.new_event_loop()
        self.executor = ThreadPoolExecutor(max_workers=1)
        
        # Create UI elements
        self._create_widgets()
        
        # Initialize plotter
        self.plotter: PolarPlotter = PolarPlotter(self.canvas, 600, 500)
        
        # Initialize async serial processor
        self.processor: Optional[AsyncSerialProcessor] = None
        
    def _update_port_list(self) -> None:
        """Update the list of available serial ports."""
        ports: List[str] = self._get_available_ports()
        if ports:
            self.port_menu['menu'].delete(0, 'end')
            for port in ports:
                self.port_menu['menu'].add_command(label=port, command=tk._setit(self.port_var, port))
            self.port_var.set(ports[0])

    @staticmethod
    def _get_available_ports() -> List[str]:
        """Get list of available serial ports."""
        ports: List[str] = []
        try:
            from serial.tools import list_ports
            ports = [port.device for port in list_ports.comports()]
        except ImportError:
            print("pyserial-tools not installed, using default port")
            ports = [DEFAULT_COM_NAME]
        return ports if ports else [DEFAULT_COM_NAME]

    def _create_widgets(self) -> None:
        """Create UI widgets."""
        # Control frame
        control_frame: tk.Frame = tk.Frame(self.root)
        control_frame.pack(pady=10)
        
        # Available ports dropdown
        self.port_var: tk.StringVar = tk.StringVar(value=DEFAULT_COM_NAME)
        self.port_menu: tk.OptionMenu = tk.OptionMenu(control_frame, self.port_var, DEFAULT_COM_NAME)
        self.port_menu.pack(side=tk.LEFT, padx=5)
        self._update_port_list()
        
        # Baudrate entry
        tk.Label(control_frame, text="Baudrate:").pack(side=tk.LEFT)
        self.baudrate_entry: tk.Entry = tk.Entry(control_frame, width=10)
        self.baudrate_entry.insert(0, str(DEFAULT_COM_BAUD))  # Default value
        self.baudrate_entry.pack(side=tk.LEFT, padx=5)

        # Connect button
        self.connect_button: tk.Button = tk.Button(
            control_frame, text="Connect", command=self._toggle_connection
        )
        self.connect_button.pack(side=tk.LEFT, padx=5)
        
        # Canvas for polar plot
        self.canvas: tk.Canvas = tk.Canvas(
            self.root, width=600, height=500, bg='white'
        )
        self.canvas.pack(pady=10)
        
    def _toggle_connection(self) -> None:
        """Toggle async serial connection."""
        if self.processor and self.processor.running:
            # Disconnect
            asyncio.run_coroutine_threadsafe(
                self.processor.disconnect(), 
                self.loop
            ).result()
            self.processor = None
            self.connect_button.config(text="Connect")
        else:
            # Connect
            port: str = self.port_var.get()
            try:
                baudrate: int = int(self.baudrate_entry.get())
                self.processor = AsyncSerialProcessor(self.plotter)
                
                # Run the connection in a separate thread with its own event loop
                future = asyncio.run_coroutine_threadsafe(
                    self._connect_and_start(port, baudrate), 
                    self.loop
                )
                
                # Wait for connection result
                connected = future.result(timeout=5)  # 5 second timeout
                
                if connected:
                    self.processor.start_reading()
                    self.connect_button.config(text="Disconnect")
                else:
                    self.processor = None
                    print("Connection failed")
                    
            except ValueError:
                print("Invalid baudrate")
            except Exception as e:
                print(f"Error connecting: {e}")
                
    async def _connect_and_start(self, port: str, baudrate: int) -> bool:
        """Connect to serial port and start reading."""
        connected = await self.processor.connect(port, baudrate)
        if connected:
            self.processor.start_reading()
        return connected

    def run(self):
        """Run the Tkinter mainloop in one thread and asyncio event loop in another."""
        # Start the asyncio event loop in a separate thread
        def run_async_loop():
            asyncio.set_event_loop(self.loop)
            self.loop.run_forever()
        
        import threading
        async_thread = threading.Thread(target=run_async_loop, daemon=True)
        async_thread.start()
        
        # Run the tkinter mainloop in the main thread
        try:
            self.root.mainloop()
        finally:
            # Clean shutdown
            self.loop.call_soon_threadsafe(self.loop.stop)
            self.executor.shutdown(wait=True)


def main() -> None:
    """Main function to run the application."""
    root: tk.Tk = tk.Tk()
    app: AsyncSerialPolarApp = AsyncSerialPolarApp(root)
    app.run()


if __name__ == "__main__":
    main()