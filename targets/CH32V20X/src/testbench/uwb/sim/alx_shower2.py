import serial
import tkinter as tk
import math
from typing import List, Optional, Tuple
import threading

DEFAULT_COM_NAME:str = "COM35"
DEFAULT_COM_BAUD:int = 115200 * 2

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
            r: Radial distance (normalized to max_radius)
            theta: Angle in radians
            color: Color of the point
        """

        # print(x,y)
        x = x*50 + self.center_x
        y = -y*50 + self.center_y
        # Draw point
        self.canvas.create_oval(
            x - 7, y - 7, x + 7, y + 7,
            fill=color, tags="point"
        )

class SerialProcessor:
    """
    Process serial data and update polar plot.
    """
    def __init__(self, port: str, baudrate: int, plotter: PolarPlotter) -> None:
        self.port: str = port
        self.baudrate: int = baudrate
        self.plotter: PolarPlotter = plotter
        self.serial_conn: Optional[serial.Serial] = None
        self.running: bool = False
        
    def connect(self) -> bool:
        """Establish serial connection."""
        try:
            self.serial_conn = serial.Serial(self.port, self.baudrate, timeout=1)
            return True
        except Exception as e:
            print(f"Failed to connect to {self.port}: {e}")
            return False
            
    def disconnect(self) -> None:
        """Close serial connection."""
        self.running = False
        if self.serial_conn and self.serial_conn.is_open:
            self.serial_conn.close()
            
    def start_reading(self) -> None:
        """Start reading data from serial port in a separate thread."""
        if not self.serial_conn or not self.serial_conn.is_open:
            if not self.connect():
                return
                
        self.running = True
        thread = threading.Thread(target=self._read_serial_data)
        thread.daemon = True
        thread.start()
        
    def _read_serial_data(self) -> None:
        """Read and process data from serial port."""
        buffer: str = ""
        
        while self.running:
            try:
                if self.serial_conn and self.serial_conn.in_waiting > 0:
                    # Read available data
                    raw_bytes: bytes = self.serial_conn.read(self.serial_conn.in_waiting)
                    # Handle decoding errors gracefully
                    buffer += raw_bytes.decode('utf-8', errors='ignore')
                    
                    # Process complete lines
                    while '\n' in buffer:
                        line, buffer = buffer.split('\n', 1)
                        self._process_line(line.strip())
                        
            except Exception as e:
                print(f"Error reading serial data: {e}")
    def _process_line(self, line: str) -> None:
        """Process a single line of data."""
        if not line:
            return
            
        try:
            # Split by comma and convert to floats
            values: List[str] = line.split(',')
            float_values: List[float] = [float(val.strip()) for val in values]
            # We need at least 2 values for (r, theta)
            if len(float_values) >= 6:
                # Update plot in main thread
                self.plotter.clear_points()
                self.plotter.plot_point_xy(float_values[0], float_values[1], "red")
                self.plotter.plot_point_xy(float_values[2], float_values[3], "blue")
                self.plotter.plot_point_xy(float_values[4], float_values[5], "pink")
                self.plotter.plot_point_xy(float_values[6], float_values[7], "orange")
                
                # print(float_values)
                
        except ValueError as e:
            print(f"Error parsing line '{line}': {e}")

class SerialPolarApp:
    """
    Main application class for the serial polar plotter.
    """
    def __init__(self, root: tk.Tk) -> None:
        self.root: tk.Tk = root
        self.root.title("Serial Polar Plotter")
        self.root.geometry("800x600")
        
        # Create UI elements
        self._create_widgets()
        
        # Initialize plotter
        self.plotter: PolarPlotter = PolarPlotter(self.canvas, 600, 500)
        
        # Initialize serial processor
        self.processor: Optional[SerialProcessor] = None
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
        
        # # Port entry
        # tk.Label(control_frame, text="Port:").pack(side=tk.LEFT)
        # self.port_entry: tk.Entry = tk.Entry(control_frame, width=10)
        # self.port_entry.insert(0, DEFAULT_COM_NAME)  # Default value
        # self.port_entry.pack(side=tk.LEFT, padx=5)
        
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
        """Toggle serial connection."""
        if self.processor and self.processor.running:
            # Disconnect
            self.processor.disconnect()
            self.processor = None
            self.connect_button.config(text="Connect")
        else:
            # Connect
            port: str = self.port_var.get()  # Changed from self.port_entry.get()
            try:
                baudrate: int = int(self.baudrate_entry.get())
                self.processor = SerialProcessor(port, baudrate, self.plotter)
                self.processor.start_reading()
                self.connect_button.config(text="Disconnect")
            except ValueError:
                print("Invalid baudrate")

def main() -> None:
    """Main function to run the application."""
    root: tk.Tk = tk.Tk()
    app: SerialPolarApp = SerialPolarApp(root)
    root.mainloop()

if __name__ == "__main__":
    main()