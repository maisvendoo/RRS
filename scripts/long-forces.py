import pandas as pd
import matplotlib.pyplot as plt

# Каталог, содержащий обрабатываемые данные
data_folder = "../../graphs/"

# Читаем данные из файла в виде таблицы
df = pd.read_csv(data_folder + "long_forces.txt", sep="\s+", header=None)

vehicle_idx = 50
force_idx = 2*vehicle_idx
motion_idx = 2*vehicle_idx - 1


plt.plot(df[0], df[force_idx], color='r')
plt.grid(linestyle='--', linewidth=0.5)
plt.xlabel('Время, с')
plt.ylabel('Усилие в сцепке локомотива, кН')

f1 = plt.gcf()

f1.savefig(data_folder + "force.png", dpi=300)

plt.show()


plt.plot(df[motion_idx], df[force_idx], color='r')
plt.grid(linestyle='--', linewidth=0.5)
plt.xlabel('Ход поглощающего аппарата, мм')
plt.ylabel('Усилие в сцепке локомотива, кН')

f2 = plt.gcf()

f2.savefig(data_folder + "foce-motion.png", dpi=300)

plt.show()

sec_time = 656.0

row_idx = int(sec_time * 100)

data_row = df.iloc[row_idx,1:]

element_idx = 1
vehicle_idx = 1
forces = []
vehicles = []

for value in data_row:
    print(value)
    
    if element_idx % 2 == 0:
        forces.append(value)
        vehicles.append(vehicle_idx)
        vehicle_idx = vehicle_idx + 1
        
    element_idx = element_idx + 1
    pass

plt.plot(vehicles, forces, color='b')
plt.grid(linestyle='--', linewidth=0.5)
plt.xlabel('Номер вагона')
plt.ylabel('Усилие в сцепке, кН')
plt.title('Распределение продольных сил в поезде при торможении')


f3 = plt.gcf()
f3.set_figwidth(16)
f3.set_figheight(8)

f3.savefig(data_folder + "forces_diagram.png", dpi=300)

plt.show()










