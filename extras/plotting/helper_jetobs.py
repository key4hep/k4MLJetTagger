import numpy as np
import matplotlib.pyplot as plt

# set the style
from cycler import cycler
import matplotlib

plt.rc('axes', prop_cycle=cycler('color', ['#73b06f', '#e6701b', '#007090', '#e7298a', '#802392', '#ffad08', '#56b4e9']))
#matplotlib.rcParams["text.usetex"] = True
matplotlib.rcParams["font.family"] = "serif"

# Set font sizes
matplotlib.rcParams.update(
    {
        "font.size": 14,  # General font size
    }
)

# helpers

def plot_kinematics(df1, df2, figsize=(12, 7), bins=50, l1='df1', l2='df2'):

    keys = ['pfcand_erel_log', 'pfcand_thetarel', 'pfcand_phirel', 'pfcand_e', 'pfcand_p']

    fig, axs = plt.subplots(2, 3, figsize=figsize)
    
    for i, ax in enumerate(axs.flat):
        if i == 5: 
            continue
        key = keys[i]
        if key == 'pfcand_e' or key == 'pfcand_p':
            ax.set_yscale('log')
        ax.hist(np.concatenate(df1[key]), bins=bins, histtype='step', label=l1, density=True, linewidth=2.0)
        ax.hist(np.concatenate(df2[key]), bins=bins, histtype='step', label=l2, density=True, linewidth=2.0)
        ax.set_xlabel(key)
        ax.legend()
        ax.grid(True)
        ax.legend()

    plt.tight_layout()
    plt.show()

def plot_pidflags(df1, df2, figsize=(12, 7), bins=10, l1='df1', l2='df2'):
    keys = ['pfcand_type', 'pfcand_charge', 'pfcand_isEl', 'pfcand_isMu', 'pfcand_isGamma', 'pfcand_isChargedHad', 'pfcand_isNeutralHad']

    fig, axs = plt.subplots(2, 4, figsize=figsize)

    for i, ax in enumerate(axs.flat):
        if i == 7:
            continue
        key = keys[i]
        ax.hist(np.concatenate(df1[key]), bins=bins, histtype='step', label=l1, density=True, linewidth=2.0)
        ax.hist(np.concatenate(df2[key]), bins=bins, histtype='step', label=l2, density=True, linewidth=2.0)
        ax.set_xlabel(key)
        ax.legend()
        ax.grid(True)

    plt.tight_layout()
    plt.show()

# retreive charged particles

def get_index_charged_particles(df, ptype='cpart', fastsim=False):
    """Choose particle type: cpart, npart, photon"""
    types = df['pfcand_type']
    if ptype=='cpart':
        num = [-211, -13, -11, 11, 13, 211] 
    elif ptype=='npart':
        num = [2112]
    elif ptype=='photon':
        num = [22]
    # fastsim is different... overwrite in that case
    if fastsim:
        if ptype=='cpart':
            num = [0]
        elif ptype=='npart':
            num = [130]
    mask_bool_chad = []
    for i in range(types.shape[0]):
        index = np.where(np.isin(types[i], num))[0]
        bool_list = np.zeros(len(types[i]), dtype=bool)
        bool_list[index] = True
        mask_bool_chad.append(bool_list)
    return mask_bool_chad

def get_value_highest_energy_particle(df, k=3, ptype='cpart', fastsim=False):
    """return the index of the k highest energy charged particle in each event"""
    particles_e = df['pfcand_e']
    mask_chad = get_index_charged_particles(df, ptype=ptype, fastsim=fastsim)
    index_charged = []
    for i  in range(particles_e.shape[0]):
        part_e = particles_e[i]
        ind = np.argsort(part_e)[::-1]
        mask = mask_chad[i]
        c_index = np.arange(len(part_e))[mask] # indicies with charged/neutral particles
        # now order the charged/neutral particles by energy with ind
        index_map = {value: np.where(ind == value)[0][0] for value in c_index}
        sorted_c_index = sorted(c_index, key=lambda x: index_map[x])
        index_charged.append(sorted_c_index[:k])
    return index_charged

def get_chad_elements(df_value, index):
    e_1 = []
    e_2 = []
    e_3 = []
    for i in range(len(df_value)):
        index_list = index[i] # this can have 1, 2 or 3 elements
        if len(index_list) == 1:
            e_1.append(df_value[i][index_list[0]])
        elif len(index_list) == 2:
            e_1.append(df_value[i][index_list[0]])
            e_2.append(df_value[i][index_list[1]])
        elif len(index_list) == 3:
            e_1.append(df_value[i][index_list[0]])
            e_2.append(df_value[i][index_list[1]])
            e_3.append(df_value[i][index_list[2]])
    return [np.array(e_1), np.array(e_2), np.array(e_3)]

# plots for charged particles 

def plot_IP(df1, df2, index1, index2, l1='df1', l2='df2', bins=70):
    keys = ['pfcand_d0', 'pfcand_z0', 'pfcand_Sip2dVal', 'pfcand_Sip2dSig', 'pfcand_Sip3dVal', 'pfcand_Sip3dSig', 'pfcand_JetDistVal', 'pfcand_JetDistSig']
    r = {'pfcand_d0': [-1, 1],
        'pfcand_z0': [-1, 1],
        'pfcand_Sip2dVal': [-1.5, 1.5],
        'pfcand_Sip2dSig': [-50, 50],
        'pfcand_Sip3dVal': [-1.5, 1.5],
        'pfcand_Sip3dSig': [-50, 50],
        'pfcand_JetDistVal': [-0.5, 0.5],
        'pfcand_JetDistSig': [-50, 50]}
    
    fig, axs = plt.subplots(4, 2, figsize=(12, 8), constrained_layout=True)
    
    for i, ax in enumerate(axs.flat):

        key = keys[i]

        # get charged particles 
        a_list = get_chad_elements(df1[key], index1)
        b_list = get_chad_elements(df2[key], index2)

        ri = r[key] if r != None else None
        # get the higest energy charged particles
        j = 0     
        a = a_list[j]
        b = b_list[j]

        if ri == None:
            combined_min = min(a.min(), b.min())
            combined_max = max(a.max(), b.max())
            combined_range = (combined_min, combined_max)
        else:
            combined_range = ri
            # make overflow bins
            a = np.clip(a, *combined_range)
            b = np.clip(b, *combined_range)

        # Plot histograms with overflow
        ax.hist(a, bins=bins, range=combined_range, label=l1, histtype='step', linewidth=2.0, density=True)
        ax.hist(b, bins=bins, range=combined_range, label=l2, histtype='step', linewidth=2.0, density=True)

        ax.legend()
        ax.grid()
        ax.set_xlabel(key)
        ax.set_yscale('log')
    
    plt.show()

def plot_cov_matrix(df1, df2, index1, index2, l1='df1', l2='df2', bins=50):
    keys = [
        'pfcand_cov_d0d0',
        'pfcand_cov_phid0',
        'pfcand_cov_d0omega',
        'pfcand_cov_omegaomega',
        'pfcand_cov_tanLambdaz0',
        'pfcand_cov_phiomega',
        'pfcand_cov_phiphi',
        'pfcand_cov_phitanLambda',
        'pfcand_cov_phiz0',
        'pfcand_cov_z0z0',
        'pfcand_cov_d0z0',
        'pfcand_cov_omegaz0',
        'pfcand_cov_tanLambdatanLambda',
        'pfcand_cov_d0tanLambda',
        'pfcand_cov_omegatanLambda',
    ]

    xlabels = {
        'pfcand_cov_omegatanLambda': r"cov($\omega$, $\tan{\lambda}$)",
        'pfcand_cov_omegaz0': r"cov($\omega$, $z_0$)",
        'pfcand_cov_tanLambdatanLambda': r"cov($\tan{\lambda}$, $\tan{\lambda}$)",
        'pfcand_cov_tanLambdaz0': r"cov($\tan{\lambda}$, $z_0$)",
        'pfcand_cov_phiomega': r"cov($\phi$, $\omega$)",
        'pfcand_cov_phitanLambda': r"cov($\phi$, $\tan{\lambda}$)",
        'pfcand_cov_phiz0': r"cov($\phi$, $z_0$)",
        'pfcand_cov_phiphi': r"cov($\phi$, $\phi$)",
        'pfcand_cov_phid0': r"cov($\phi$, $d_0$)",
        'pfcand_cov_omegaomega': r"cov($\omega$, $\omega$)",
        'pfcand_cov_d0omega': r"cov($d_0$, $\omega$)",
        'pfcand_cov_d0tanLambda': r"cov($d_0$, $\tan{\lambda}$)",
        'pfcand_cov_d0d0': r"cov($d_0$, $d_0$)",
        'pfcand_cov_d0z0': r"cov($d_0$, $z_0$)",
        'pfcand_cov_z0z0': r"cov($z_0$, $z_0$)",
    }

    r = {
        'pfcand_cov_omegatanLambda': [-1e-11, 1e-11],
        'pfcand_cov_omegaz0': [-2e-10, 2e-10],
        'pfcand_cov_tanLambdatanLambda': [0, 0.5e-6],
        'pfcand_cov_tanLambdaz0': [-0.5e-5, 0],
        'pfcand_cov_phiomega': [0, 0.2e-10],
        'pfcand_cov_phitanLambda': [-0.2e-08, 0.2e-08], 
        'pfcand_cov_phiz0': [-0.05e-6, 0.05e-6],
        'pfcand_cov_phiphi': [0.0, 0.3e-6], 
        'pfcand_cov_phid0': [-0.3e-5, 0],
        'pfcand_cov_omegaomega': [0, 5e-13], #[0, 0.5e-13], 
        'pfcand_cov_d0omega': [-0.25e-9, 0.25e-9],
        'pfcand_cov_d0tanLambda': [-0.1e-6, 0.1e-6], 
        'pfcand_cov_d0d0': [0, 8e-5],
        'pfcand_cov_d0z0': [-0.25e-5, 0.25e-5], # here
        'pfcand_cov_z0z0': [0, 9e-5],
    }


    fig, axs = plt.subplots(5, 3, figsize=(12, 15), constrained_layout=True)

    list_for_pairs = [[df1, index1, l1], [df2, index2, l2]]

    for k in range(2):
        for i, key in enumerate(keys):
            if k == "full":
                d = get_chad_elements(dic[k][key], index_charged_3E_full)[0]
            d = get_chad_elements(list_for_pairs[k][0][key], list_for_pairs[k][1])[0] 
            d = np.clip(d, r[key][0], r[key][1])
            ax = axs.ravel()
            ax[i].hist(d, bins=bins, histtype='step', label=list_for_pairs[k][2], density=True, linewidth=2.0, range=r[key])
            ax[i].set_xlabel(xlabels[key])
            ax[i].legend()
            ax[i].grid(True)
    plt.show()
