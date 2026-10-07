#!/usr/bin/env python3
"""
Zuma Deluxe — Graphical Port Launcher & Builder for Miyoo Mini / OnionOS
========================================================================
A lightweight, user-friendly GUI to select your original PC Zuma Deluxe
folder, choose the output destination, view live build progress, and
generate a ready-to-flash SD card ZIP archive in seconds.

Requires Python 3.8+ (standard library tkinter). Zero pip dependencies needed.
"""

import os
import sys
import threading
import subprocess
import platform

import tkinter as tk
from tkinter import ttk, filedialog, messagebox, scrolledtext

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(SCRIPT_DIR)
sys.path.insert(0, SCRIPT_DIR)

from build_release import build_package, REQUIRED_ASSET_DIRS

def detect_steam_path():
    """Try to find the default Zuma Deluxe installation."""
    candidates = [
        os.path.join(REPO_ROOT, "Zuma Deluxe"),
        os.path.join(REPO_ROOT, "..", "Zuma Deluxe"),
        r"C:\Program Files (x86)\Steam\steamapps\common\Zuma Deluxe",
        r"C:\Program Files\Steam\steamapps\common\Zuma Deluxe",
        r"C:\PopCap Games\Zuma Deluxe",
        r"D:\SteamLibrary\steamapps\common\Zuma Deluxe",
        r"E:\SteamLibrary\steamapps\common\Zuma Deluxe",
        os.path.expanduser("~/.steam/steam/steamapps/common/Zuma Deluxe"),
        os.path.expanduser("~/Library/Application Support/Steam/steamapps/common/Zuma Deluxe")
    ]
    for c in candidates:
        if os.path.isdir(c) and all(os.path.isdir(os.path.join(c, d)) for d in ["images", "sounds"]):
            return os.path.abspath(c)
    return ""

class ZumaPortLauncherGUI(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title("Zuma Deluxe — Miyoo Mini Port Builder")
        self.geometry("720x620")
        self.minsize(640, 540)

        # Set window icon if available
        icon_path = os.path.join(REPO_ROOT, "icon.png")
        if os.path.isfile(icon_path):
            try:
                img = tk.PhotoImage(file=icon_path)
                self.iconphoto(False, img)
            except Exception:
                pass

        self.style = ttk.Style(self)
        # Try modern theme if available
        available_themes = self.style.theme_names()
        if "clam" in available_themes:
            self.style.theme_use("clam")

        self.configure_styles()
        self.create_widgets()
        self.auto_detect_game()

    def configure_styles(self):
        self.style.configure("Title.TLabel", font=("Helvetica", 14, "bold"))
        self.style.configure("Subtitle.TLabel", font=("Helvetica", 9), foreground="#555555")
        self.style.configure("Header.TLabel", font=("Helvetica", 10, "bold"))
        self.style.configure("Status.TLabel", font=("Helvetica", 9, "bold"))
        self.style.configure("Build.TButton", font=("Helvetica", 11, "bold"), padding=6)
        self.style.configure("Action.TButton", font=("Helvetica", 9), padding=4)

    def create_widgets(self):
        main_frame = ttk.Frame(self, padding="15")
        main_frame.pack(fill=tk.BOTH, expand=True)

        # Header Banner
        header_frame = ttk.Frame(main_frame)
        header_frame.pack(fill=tk.X, pady=(0, 15))

        title_lbl = ttk.Label(header_frame, text="Zuma Deluxe — Miyoo Mini Port Builder", style="Title.TLabel")
        title_lbl.pack(anchor=tk.W)

        subtitle_lbl = ttk.Label(
            header_frame,
            text="Generate a ready-to-play OnionOS package from your legally owned PC game.",
            style="Subtitle.TLabel"
        )
        subtitle_lbl.pack(anchor=tk.W, pady=(2, 0))

        # Section 1: PC Game Path
        sec1 = ttk.LabelFrame(main_frame, text=" 1. Original PC Game Folder ", padding="10")
        sec1.pack(fill=tk.X, pady=(0, 12))

        lbl_s = ttk.Label(sec1, text="Select the directory where Zuma Deluxe for PC is installed (Steam, CD, EA App):")
        lbl_s.pack(anchor=tk.W, pady=(0, 4))

        f_s = ttk.Frame(sec1)
        f_s.pack(fill=tk.X)

        self.var_source = tk.StringVar()
        self.entry_source = ttk.Entry(f_s, textvariable=self.var_source)
        self.entry_source.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 6))

        btn_browse_s = ttk.Button(f_s, text="Browse...", style="Action.TButton", command=self.browse_source)
        btn_browse_s.pack(side=tk.LEFT, padx=(0, 4))

        btn_detect = ttk.Button(f_s, text="Auto-Detect", style="Action.TButton", command=self.auto_detect_game)
        btn_detect.pack(side=tk.LEFT)

        # Section 2: Output Destination
        sec2 = ttk.LabelFrame(main_frame, text=" 2. Output Destination ", padding="10")
        sec2.pack(fill=tk.X, pady=(0, 12))

        lbl_o = ttk.Label(sec2, text="Folder where the ready-to-flash 'Zuma_Deluxe_MiyooMini.zip' will be created:")
        lbl_o.pack(anchor=tk.W, pady=(0, 4))

        f_o = ttk.Frame(sec2)
        f_o.pack(fill=tk.X)

        default_dist = os.path.join(REPO_ROOT, "dist")
        self.var_output = tk.StringVar(value=default_dist)
        self.entry_output = ttk.Entry(f_o, textvariable=self.var_output)
        self.entry_output.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 6))

        btn_browse_o = ttk.Button(f_o, text="Browse...", style="Action.TButton", command=self.browse_output)
        btn_browse_o.pack(side=tk.LEFT)

        # Section 3: Engine Mode
        sec3 = ttk.LabelFrame(main_frame, text=" 3. Port Engine Mode ", padding="10")
        sec3.pack(fill=tk.X, pady=(0, 12))

        self.var_docker_build = tk.BooleanVar(value=False)
        lbl_engine_info = ttk.Label(
            sec3,
            text="✓ Standalone Mode: Uses the included precompiled engine (bin/Zuma).\n   No Docker, compilers, or pip packages required!",
            font=("Helvetica", 9),
            foreground="#2e7d32"
        )
        lbl_engine_info.pack(anchor=tk.W)

        # Section 4: Progress Bar & Status
        sec4 = ttk.Frame(main_frame)
        sec4.pack(fill=tk.X, pady=(0, 8))

        f_status = ttk.Frame(sec4)
        f_status.pack(fill=tk.X, pady=(0, 4))

        self.var_status = tk.StringVar(value="Status: Ready to build")
        lbl_status = ttk.Label(f_status, textvariable=self.var_status, style="Status.TLabel")
        lbl_status.pack(side=tk.LEFT)

        self.var_pct = tk.StringVar(value="0%")
        lbl_pct = ttk.Label(f_status, textvariable=self.var_pct, style="Status.TLabel")
        lbl_pct.pack(side=tk.RIGHT)

        self.prog_bar = ttk.Progressbar(sec4, orient=tk.HORIZONTAL, mode="determinate")
        self.prog_bar.pack(fill=tk.X)

        # Section 5: Console / Log Area
        lbl_log = ttk.Label(main_frame, text="Build Output & Logs:", font=("Helvetica", 9))
        lbl_log.pack(anchor=tk.W, pady=(4, 2))

        self.log_text = scrolledtext.ScrolledText(
            main_frame,
            height=7,
            bg="#1e1e1e",
            fg="#dcdcdc",
            insertbackground="white",
            font=("Consolas", 9),
            wrap=tk.WORD
        )
        self.log_text.pack(fill=tk.BOTH, expand=True, pady=(0, 12))

        # Section 6: Action Buttons
        btn_frame = ttk.Frame(main_frame)
        btn_frame.pack(fill=tk.X)

        self.btn_build = ttk.Button(
            btn_frame,
            text="🚀  Build Port Package (ZIP)",
            style="Build.TButton",
            command=self.start_build
        )
        self.btn_build.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 8))

        self.btn_open_folder = ttk.Button(
            btn_frame,
            text="📁  Open Output Folder",
            style="Action.TButton",
            state=tk.DISABLED,
            command=self.open_output_dir
        )
        self.btn_open_folder.pack(side=tk.RIGHT)

        self.log("Welcome to Zuma Deluxe Miyoo Mini Port Builder!")
        self.log("Please select your PC Zuma Deluxe folder and click 'Build Port Package'.")

    def log(self, message):
        """Append log message in the console text box safely."""
        def _append():
            self.log_text.insert(tk.END, message + "\n")
            self.log_text.see(tk.END)
        self.after(0, _append)

    def auto_detect_game(self):
        detected = detect_steam_path()
        if detected:
            self.var_source.set(detected)
            self.log(f"Auto-detected Zuma Deluxe installation at: {detected}")
        else:
            self.log("Could not auto-detect Zuma Deluxe installation. Please browse manually.")

    def browse_source(self):
        folder = filedialog.askdirectory(
            title="Select Zuma Deluxe PC Installation Folder",
            initialdir=self.var_source.get() or "C:\\"
        )
        if folder:
            self.var_source.set(os.path.normpath(folder))
            self.log(f"Selected game folder: {folder}")

    def browse_output(self):
        folder = filedialog.askdirectory(
            title="Select Output Destination Directory",
            initialdir=self.var_output.get() or REPO_ROOT
        )
        if folder:
            self.var_output.set(os.path.normpath(folder))
            self.log(f"Selected output folder: {folder}")

    def update_progress(self, pct, message):
        def _update():
            self.prog_bar["value"] = pct
            self.var_pct.set(f"{pct}%")
            self.var_status.set(f"Status: {message}")
        self.after(0, _update)

    def start_build(self):
        source = self.var_source.get().strip()
        output_dir = self.var_output.get().strip()
        compile_source = self.var_docker_build.get()

        if not source or not os.path.isdir(source):
            messagebox.showerror(
                "Invalid Source Folder",
                "Please select a valid directory where Zuma Deluxe for PC is installed."
            )
            return

        # Validate required directories in source
        missing = [d for d in REQUIRED_ASSET_DIRS if not os.path.isdir(os.path.join(source, d))]
        if missing:
            messagebox.showerror(
                "Missing Game Assets",
                f"The selected directory is missing required Zuma Deluxe game folders:\n{missing}\n\n"
                "Please make sure you selected the main game directory (e.g., Steam common/Zuma Deluxe)."
            )
            return

        if not output_dir:
            output_dir = os.path.join(REPO_ROOT, "dist")
            self.var_output.set(output_dir)

        # Safety Check: Source and Output must NOT be identical
        if os.path.normcase(os.path.abspath(source)) == os.path.normcase(os.path.abspath(output_dir)):
            messagebox.showerror(
                "Folder Conflict",
                "Source folder and Output destination cannot be the same directory!\n\n"
                "Please choose a separate folder for your output (such as the default 'dist' folder)."
            )
            return

        # Safety Check: Do not output directly into repository root
        if os.path.normcase(os.path.abspath(output_dir)) == os.path.normcase(os.path.abspath(REPO_ROOT)):
            output_dir = os.path.join(REPO_ROOT, "dist")
            self.var_output.set(output_dir)
            self.log("Note: Output redirected to 'dist' folder to protect repository source files.")

        self.btn_build.config(state=tk.DISABLED)
        self.btn_open_folder.config(state=tk.DISABLED)
        self.prog_bar["value"] = 0
        self.var_pct.set("0%")
        self.var_status.set("Status: Starting build...")
        self.log("\n" + "="*60)
        self.log(f"Starting build from: {source}")
        self.log(f"Destination: {output_dir}")
        self.log("="*60)

        # Run build in a separate thread so GUI remains responsive
        thread = threading.Thread(
            target=self._worker_build,
            args=(source, output_dir, compile_source),
            daemon=True
        )
        thread.start()

    def _worker_build(self, source, output_dir, compile_source):
        try:
            zip_path, dist_dir = build_package(
                source=source,
                output_dir=output_dir,
                build_from_source=compile_source,
                progress_cb=self.update_progress,
                log_cb=self.log
            )

            def _on_success():
                self.btn_build.config(state=tk.NORMAL)
                self.btn_open_folder.config(state=tk.NORMAL)
                self.output_dist_path = dist_dir
                messagebox.showinfo(
                    "Build Complete!",
                    f"Zuma Deluxe port package has been built successfully!\n\n"
                    f"Archive:\n{zip_path}\n\n"
                    "Next Steps:\n"
                    "1. Extract the contents of the ZIP archive to the ROOT of your Miyoo Mini microSD card.\n"
                    "2. Insert the card into your console and enjoy!"
                )

            self.after(0, _on_success)

        except Exception as e:
            def _on_error(err=str(e)):
                self.btn_build.config(state=tk.NORMAL)
                self.var_status.set("Status: Build failed.")
                self.log(f"\nERROR: {err}")
                if "used by another process" in err.lower() or "otro proceso" in err.lower() or "permission denied" in err.lower():
                    messagebox.showerror(
                        "File In Use / Archivo en uso",
                        f"A file or folder in the destination is currently locked by another program:\n\n{err}\n\n"
                        "Please close any Windows Explorer windows or ZIP viewers open to that folder and try again."
                    )
                else:
                    messagebox.showerror("Build Failed", f"An error occurred while building the port package:\n\n{err}")

            self.after(0, _on_error)

    def open_output_dir(self):
        path = getattr(self, "output_dist_path", self.var_output.get())
        if os.path.exists(path):
            if platform.system() == "Windows":
                os.startfile(path)
            elif platform.system() == "Darwin":
                subprocess.run(["open", path])
            else:
                subprocess.run(["xdg-open", path])
        else:
            messagebox.showwarning("Folder Not Found", f"The folder does not exist yet:\n{path}")

def main():
    app = ZumaPortLauncherGUI()
    app.mainloop()

if __name__ == "__main__":
    main()
