import pandas as pd
import matplotlib.pyplot as plt
from tkinter import Tk, Label, Button, filedialog, messagebox, Listbox, Entry, StringVar, Toplevel, END, Scrollbar, Frame
from datetime import datetime


class TrafficAnalyzerApp:
    def __init__(self, master):
        self.master = master
        self.master.title("Traffic Data Analyzer")
        self.master.geometry("600x600")  # Resize window
        self.master.config(bg="#f0f0f0")  # Set background color

        self.df = None

        # Create a Frame for the title
        header_frame = Frame(master, bg="#007acc", pady=20)
        header_frame.pack(fill="both")
        
        Label(header_frame, text="Traffic Data Analyzer", font=("Arial", 18, "bold"), fg="white", bg="#007acc").pack()

        # Create a Frame for the buttons
        button_frame = Frame(master, bg="#f0f0f0", pady=20)
        button_frame.pack(fill="both")

        Button(button_frame, text="Load Data", command=self.load_data, width=30, height=2, bg="#4CAF50", fg="white", font=("Arial", 12)).pack(pady=5)
        Button(button_frame, text="View Traffic Data in Time Range", command=self.view_in_time_range, width=30, height=2, bg="#4CAF50", fg="white", font=("Arial", 12)).pack(pady=5)
        Button(button_frame, text="Analyze Average Traffic", command=self.analyze_average_traffic, width=30, height=2, bg="#4CAF50", fg="white", font=("Arial", 12)).pack(pady=5)
        Button(button_frame, text="Find Peak Traffic Hour", command=self.find_peak_traffic_hour, width=30, height=2, bg="#4CAF50", fg="white", font=("Arial", 12)).pack(pady=5)
        Button(button_frame, text="Add Traffic Data", command=self.add_traffic_data, width=30, height=2, bg="#4CAF50", fg="white", font=("Arial", 12)).pack(pady=5)
        Button(button_frame, text="Delete Traffic Data", command=self.delete_traffic_data, width=30, height=2, bg="#4CAF50", fg="white", font=("Arial", 12)).pack(pady=5)
        Button(button_frame, text="Export Traffic Data to CSV", command=self.export_to_csv, width=30, height=2, bg="#4CAF50", fg="white", font=("Arial", 12)).pack(pady=5)
        Button(button_frame, text="Display Traffic Summary", command=self.show_summary, width=30, height=2, bg="#4CAF50", fg="white", font=("Arial", 12)).pack(pady=5)
        Button(button_frame, text="Visualize Traffic Data", command=self.plot_traffic_data, width=30, height=2, bg="#4CAF50", fg="white", font=("Arial", 12)).pack(pady=5)
        Button(button_frame, text="Exit", command=self.master.quit, width=30, height=2, bg="#f44336", fg="white", font=("Arial", 12)).pack(pady=5)

        # Create a Frame for the Listbox to display data
        list_frame = Frame(master, bg="#f0f0f0")
        list_frame.pack(fill="both", padx=10, pady=20)

        self.listbox = Listbox(list_frame, height=10, width=70, font=("Arial", 10), bg="#ffffff", selectbackground="#4CAF50")
        self.listbox.pack(side="left", fill="y")

        # Scrollbar for the Listbox
        scrollbar = Scrollbar(list_frame, orient="vertical", command=self.listbox.yview)
        scrollbar.pack(side="right", fill="y")
        self.listbox.config(yscrollcommand=scrollbar.set)

    def load_data(self):
        try:
            file_path = filedialog.askopenfilename(title="Select Traffic Data CSV",
                                                   filetypes=[("CSV Files", "*.csv")])
            if not file_path:
                return

            self.df = pd.read_csv(file_path, header=None, names=["Traffic Count", "Timestamp"])
            self.df["Timestamp"] = self.df["Timestamp"].str.strip()
            self.df["Timestamp"] = pd.to_datetime(self.df["Timestamp"], format='%a %b %d %H:%M:%S %Y')

            self.listbox.delete(0, END)
            self.listbox.insert(END, "Data loaded successfully!")
            for _, row in self.df.iterrows():
                self.listbox.insert(END, f"{row['Traffic Count']}, {row['Timestamp']}")
        except Exception as e:
            messagebox.showerror("Error", f"Error loading data: {e}")

    def view_in_time_range(self):
        if self.df is None:
            messagebox.showwarning("Warning", "Please load data first!")
            return

        range_window = Toplevel(self.master)
        range_window.title("View Traffic Data in Time Range")
        range_window.geometry("400x300")

        Label(range_window, text="Start Time (YYYY-MM-DD HH:MM:SS):").pack(pady=5)
        start_time_var = StringVar()
        Entry(range_window, textvariable=start_time_var, width=30).pack(pady=5)

        Label(range_window, text="End Time (YYYY-MM-DD HH:MM:SS):").pack(pady=5)
        end_time_var = StringVar()
        Entry(range_window, textvariable=end_time_var, width=30).pack(pady=5)

        def filter_data():
            try:
                start_time = pd.to_datetime(start_time_var.get())
                end_time = pd.to_datetime(end_time_var.get())
                filtered_df = self.df[(self.df["Timestamp"] >= start_time) & (self.df["Timestamp"] <= end_time)]

                self.listbox.delete(0, END)
                if filtered_df.empty:
                    self.listbox.insert(END, "No data found in the given time range.")
                else:
                    for _, row in filtered_df.iterrows():
                        self.listbox.insert(END, f"{row['Traffic Count']}, {row['Timestamp']}")
                range_window.destroy()
            except Exception as e:
                messagebox.showerror("Error", f"Invalid input: {e}")

        Button(range_window, text="Filter Data", command=filter_data, width=15, height=2, bg="#4CAF50", fg="white", font=("Arial", 12)).pack(pady=10)

    def analyze_average_traffic(self):
        if self.df is None:
            messagebox.showwarning("Warning", "Please load data first!")
            return

        avg_traffic = self.df["Traffic Count"].mean()
        messagebox.showinfo("Average Traffic", f"The average traffic count is: {avg_traffic:.2f}")

    def find_peak_traffic_hour(self):
        if self.df is None:
            messagebox.showwarning("Warning", "Please load data first!")
            return

        self.df["Hour"] = self.df["Timestamp"].dt.hour
        peak_hour = self.df.groupby("Hour")["Traffic Count"].sum().idxmax()
        peak_traffic = self.df.groupby("Hour")["Traffic Count"].sum().max()

        messagebox.showinfo("Peak Traffic Hour",
                            f"The peak traffic hour is: {peak_hour}:00 with {peak_traffic} vehicles.")

    def add_traffic_data(self):
        if self.df is None:
            messagebox.showwarning("Warning", "Please load data first!")
            return

        add_window = Toplevel(self.master)
        add_window.title("Add Traffic Data")
        add_window.geometry("400x300")

        Label(add_window, text="Traffic Count:").pack(pady=5)
        traffic_count_var = StringVar()
        Entry(add_window, textvariable=traffic_count_var, width=30).pack(pady=5)

        Label(add_window, text="Timestamp (YYYY-MM-DD HH:MM:SS):").pack(pady=5)
        timestamp_var = StringVar()
        Entry(add_window, textvariable=timestamp_var, width=30).pack(pady=5)

        def add_data():
            try:
                traffic_count = int(traffic_count_var.get())
                timestamp = pd.to_datetime(timestamp_var.get())
                new_row = pd.DataFrame({"Traffic Count": [traffic_count], "Timestamp": [timestamp]})
                self.df = pd.concat([self.df, new_row], ignore_index=True).sort_values(by="Timestamp")
                self.listbox.insert(END, f"{traffic_count}, {timestamp}")
                add_window.destroy()
            except Exception as e:
                messagebox.showerror("Error", f"Invalid input: {e}")

        Button(add_window, text="Add Data", command=add_data, width=15, height=2, bg="#4CAF50", fg="white", font=("Arial", 12)).pack(pady=10)

    def delete_traffic_data(self):
        if self.df is None:
            messagebox.showwarning("Warning", "Please load data first!")
            return

        selected = self.listbox.curselection()
        if not selected:
            messagebox.showwarning("Warning", "Please select a row to delete!")
            return

        index = selected[0] - 1
        if index >= 0:
            self.df = self.df.drop(index).reset_index(drop=True)
            self.listbox.delete(selected)

    def export_to_csv(self):
        if self.df is None:
            messagebox.showwarning("Warning", "Please load data first!")
            return

        file_path = filedialog.asksaveasfilename(defaultextension=".csv",
                                                 filetypes=[("CSV Files", "*.csv")])
        if not file_path:
            return

        self.df.to_csv(file_path, index=False)
        messagebox.showinfo("Export Success", f"Data exported to {file_path}")

    def show_summary(self):
        if self.df is None:
            messagebox.showwarning("Warning", "Please load data first!")
            return

        total_count = self.df["Traffic Count"].sum()
        average_count = self.df["Traffic Count"].mean()
        start_time = self.df["Timestamp"].min()
        end_time = self.df["Timestamp"].max()

        summary = (f"Total Traffic Count: {total_count}\n"
                   f"Average Traffic Count: {average_count:.2f}\n"
                   f"Start Time: {start_time}\n"
                   f"End Time: {end_time}")
        messagebox.showinfo("Summary", summary)

    def plot_traffic_data(self):
        if self.df is None:
            messagebox.showwarning("Warning", "Please load data first!")
            return

        plt.figure(figsize=(10, 5))
        plt.plot(self.df["Timestamp"], self.df["Traffic Count"], marker="o")
        plt.title("Traffic Count Over Time")
        plt.xlabel("Timestamp")
        plt.ylabel("Traffic Count")
        plt.grid()
        plt.show()


if __name__ == "__main__":
    root = Tk()
    app = TrafficAnalyzerApp(root)
    root.mainloop()
