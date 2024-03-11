import pandas as pd
import matplotlib.pyplot as plt

data_folder = "../../graphs/"

df = pd.read_csv(data_folder + "long_forces.txt", sep="\s+", header=None)


plt.plot(df[0], df[2], color='r')
plt.grid(linestyle='--', linewidth=0.5)
plt.xlabel('Время, с')
plt.ylabel('Усилие в сцепке локомотива, кН')

f1 = plt.gcf()

f1.savefig(data_folder + "force.png", dpi=300)

plt.show()


plt.plot(df[1], df[2], color='r')
plt.grid(linestyle='--', linewidth=0.5)
plt.xlabel('Ход поглощающего аппарата, мм')
plt.ylabel('Усилие в сцепке локомотива, кН')

f2 = plt.gcf()

f2.savefig(data_folder + "foce-motion.png", dpi=300)

plt.show()






