import matplotlib.pyplot as plt
from scipy.signal import cheby2


# b=[ 0.88244379 -2.63978216  2.63978216 -0.88244379]
# a=[ 1.         -2.74218054  2.52356443 -0.77870694]
# Only to demonstrate the filter computation, not efficient numpy codes.
def filter(yn, xn):
    for i in range(len(xn)):
        if i > 3:
            yn[i] = (
                0.88244379 * xn[i - 1]
                - 2.63978216 * xn[i - 2]
                + 2.63978216 * xn[i - 3]
                - 0.88244379 * xn[i - 4]
                + 2.7421805 * yn[i - 1]
                - 2.52356443 * yn[i - 2]
                + 0.77870694 * yn[i - 3]
            )


# Filter design parameters
order = 3  # 3rd order filter
fs = 500  # Sampling frequency in Hz
cutoff = 100  # Cutoff frequency in Hz

# Normalize the cutoff frequency (Nyquist = fs/2)
nyq = 0.5 * fs
normal_cutoff = cutoff / nyq

# Apply filter to a sample signal
# Create a noisy signal: 5 Hz + 250 Hz sine waves
t = np.linspace(0, 1.0, int(fs), endpoint=False)
signal = np.sin(2 * np.pi * 10 * t) + np.sin(2 * np.pi * 20 * t)

# Plot the original signal
fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True)
ax1.plot(t, signal)
ax1.set_title("10 Hz and 20 Hz sinusoids")
ax1.axis([0, 1, -2, 2])

size = len(signal)
np.savetxt(
    "signal.txt",
    signal.reshape(-1, 10),
    fmt="%.2f",
    delimiter=",",
    newline=",\n",
    header=f"signal[{size}]=",
    footer="}",
)

# Apply filter()
b, a = cheby2(order, 20, 17, "hp", fs=1000, output="ba")
zero_padding = np.zeros(3)
signal_padding = np.concatenate((zero_padding, signal))
filtered_signal_padding = np.zeros_like(signal_padding)
filter(filtered_signal_padding, signal_padding)
filtered_signal_final = filtered_signal_padding[3:]
# Save output signal
size = len(filtered_signal_final)
np.savetxt(
    "filtered_signal.txt",
    filtered_signal_final.reshape(-1, 10),
    fmt="%.2f",
    delimiter=",",
    newline=",\n",
    header=f"filtered_signal[{size}]=",
    footer="}",
)
ax2.plot(t, filtered_signal_final)
ax2.set_title("After applying filter function")
ax2.axis([0, 1, -2, 2])
ax2.set_xlabel("Time [s]")
plt.show()
