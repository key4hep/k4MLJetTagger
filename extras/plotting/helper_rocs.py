#
# Copyright (c) 2020-2024 Key4hep-Project.
#
# This file is part of Key4hep.
# See https://key4hep.github.io/key4hep-doc/ for further info.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
import uproot
import numpy as np
import matplotlib.pyplot as plt
from sklearn.metrics import roc_curve, roc_auc_score, auc
from matplotlib.lines import Line2D

# set the style
from cycler import cycler
import matplotlib
import time

plt.rc('axes', prop_cycle=cycler('color', ['#73b06f', '#e6701b', '#007090', '#e7298a', '#802392', '#ffad08', '#56b4e9']))
#matplotlib.rcParams["text.usetex"] = True
matplotlib.rcParams["font.family"] = "serif"

# Set font sizes
matplotlib.rcParams.update(
    {
        "font.size": 14,  # General font size
    }
)


# constum path for saving data

path = "./"
#path = "/afs/cern.ch/work/s/saaumill/public/FullSimTagger/notebooks/plots_from_key4hep"

def load_data(file_name, key='JetTags;1'):
    t1 = time.time()
    file = uproot.open(file_name)
    tree = file[key]
    data = tree.arrays(library="np")
    t2 = time.time()
    print(f"Time to load data: {t2-t1}")
    return data

def invalid_ind(data):
    ind = []
    labels = ['U', 'D', 'C', 'S', 'B', 'G', 'TAU']
    for l in labels:
        true_labels = data[f'recojet_is{l}']
        scores = data[f'score_recojet_is{l}']
        valid_indices = np.where(np.isnan(scores))
        ind.append(valid_indices)
    return np.unique(ind)

# binary ROC curves

# from https://github.com/doloresgarcia/LOGML_2024/blob/81dfa6428bdd6e652908562474cded362d2bf5f6/src/utils/logger_wandb.py#L204
def create_binary_rocs(positive, negative, y_true, y_score):
    mask_positive = y_true == positive
    if negative == 0 or negative == 1: # merge u and d
        mask_negative = np.logical_or(y_true == 0, y_true == 1)# is that correct???
    else:
        mask_negative = y_true == negative
    #print("mask positive shape", mask_positive.shape)
    number_positive = np.sum(mask_positive)
    #print("number positive", number_positive)
    number_negative = np.sum(mask_negative)

    if number_positive > 0 and number_negative > 0:
        # Create binary labels for positive and negative classes
        y_true_positive = np.ones(number_positive)
        y_true_negative = np.zeros(number_negative)
        y_true_ = np.concatenate([y_true_positive, y_true_negative]) # array like [1,1,1,...,0,0,0,0]

        # Select scores for positive and negative classes
        y_score_positive = y_score[mask_positive]
        y_score_negative = y_score[mask_negative]

        indices = np.array([negative, positive]) # [2,4]
        y_score_selected_positive = y_score_positive[:, indices] # from all MC positive scores (true MC b) take all probabilities for b and c
        y_score_selected_negative = y_score_negative[:, indices] # from all MC netagive scores (true MC c) talke all probabilities for b and c

        # Calculate probabilities using softmax for BINARY discriminat
        y_scores_pos_prob = y_score_selected_positive / np.sum(
            y_score_selected_positive, axis=1, keepdims=True
        )
        y_scores_neg_prob = y_score_selected_negative / np.sum(
            y_score_selected_negative, axis=1, keepdims=True
        )

        log_y_scores_pos = np.log10(y_scores_pos_prob/(1-y_scores_pos_prob))
        log_y_scores_neg = np.log10(y_scores_neg_prob/(1-y_scores_neg_prob))

        # Extract the probability for the positive class
        y_prob_positiveclass = y_scores_pos_prob[:, 1] # 1, so that b is selected
        y_prob_positiveclass_neg = y_scores_neg_prob[:, 1]



        # Concatenate probabilities for positive and negative classes
        y_prob_positive = np.concatenate([y_prob_positiveclass, y_prob_positiveclass_neg])

        y_prob_positive_log = np.concatenate([log_y_scores_pos[:,1], log_y_scores_neg[:,1]])

        # Compute ROC curve and AUC score
        #print("y_true_: ", y_true_, 'shape:', y_true_.shape) # ever entry that's a MC b quark
        #print("y_prob_positive: ", y_prob_positive, 'shape:', y_prob_positive.shape) # how high is the probability that this is a b quark? (normed by the probabitiy for being a b and c quark?)
        fpr, tpr, thrs = roc_curve(y_true_, y_prob_positive_log, pos_label=1)
        auc_score = roc_auc_score(y_true_, y_prob_positive_log)

        return [fpr, tpr, auc_score]#, y_true_, y_prob_positive, y_prob_positive_log]
    else:
        return []

def log_multiline_rocs_b(y_true, y_score, labels, ax, k,j, ls="solid", l=True):
    q_tag = 4 # b-tagging
    vs_tag = [1,2, 5] # g, u, c
    color_cycle = plt.rcParams['axes.prop_cycle'].by_key()['color']
    dic_c = {1: color_cycle[0], 2: color_cycle[1], 3: color_cycle[4], 4: color_cycle[3], 5: color_cycle[2]}
    _bg = create_binary_rocs(q_tag, vs_tag[0], y_true, y_score)
    _bud = create_binary_rocs(q_tag, vs_tag[1], y_true, y_score)
    _bc = create_binary_rocs(q_tag, vs_tag[2], y_true, y_score)


    if len(_bg) > 0 and len(_bud) > 0 and len(_bc) > 0:
        # Calculate TPR for different ROC curves (this function needs to be defined elsewhere)
        #calculate_and_log_tpr_1_10_percent(_bg[0], _bg[1], "b", "g")
        #calculate_and_log_tpr_1_10_percent(_bud[0], _bud[1], "b", "ud")
        #calculate_and_log_tpr_1_10_percent(_bc[0], _bc[1], "b", "c")

        # Plot ROC curves
        xs = [_bg[1], _bud[1], _bc[1]]
        ys = [_bg[0], _bud[0], _bc[0]]
        auc_ = [_bg[2], _bud[2], _bc[2]]

        # plot
        ys_log = [np.log10(j + 1e-8) for j in ys]
        i = 0
        for x, y in zip(xs, ys_log):
            if l:
                label = f"{labels[q_tag]} vs {labels[vs_tag[i]]}"
            else:
                label=None
            ax[k, j].plot(x, y, label=label, color=dic_c[vs_tag[i]], linestyle=ls) #(AUC={auc_[i]:.3f})")
            i += 1
        ax[k,j].legend(title=f"{labels[q_tag]}-tagging", title_fontproperties={'weight':'bold'})
        ax[k,j].grid()
        ax[k,j].set_ylim(-3, 0)
        ax[k,j].set_xlabel("jet tagging efficiency")
        ax[k,j].set_ylabel("log10(jet misid. probability)")
    else:
        print("failed")

def log_multiline_rocs_c(y_true, y_score, labels, ax, k, j, ls="solid", l=True):
    q_tag = 2 # b-tagging
    vs_tag = [1,4, 5] # ud, b, g
    color_cycle = plt.rcParams['axes.prop_cycle'].by_key()['color']
    dic_c = {1: color_cycle[0], 2: color_cycle[1], 3: color_cycle[4], 4: color_cycle[3], 5: color_cycle[2]}
    _bg = create_binary_rocs(q_tag, vs_tag[0], y_true, y_score)
    _bud = create_binary_rocs(q_tag, vs_tag[1], y_true, y_score)
    _bc = create_binary_rocs(q_tag, vs_tag[2], y_true, y_score)

    if len(_bg) > 0 and len(_bud) > 0 and len(_bc) > 0:
        xs = [_bg[1], _bud[1], _bc[1]]
        ys = [_bg[0], _bud[0], _bc[0]]
        auc_ = [_bg[2], _bud[2], _bc[2]]

        # plot
        ys_log = [np.log10(j + 1e-8) for j in ys]
        i = 0
        for x, y in zip(xs, ys_log):
            if l:
                label = f"{labels[q_tag]} vs {labels[vs_tag[i]]}"
            else:
                label=None
            ax[k, j].plot(x, y, label=label, color=dic_c[vs_tag[i]], linestyle=ls) #(AUC={auc_[i]:.3f})")
            i += 1
        ax[k, j].legend(title=f"{labels[q_tag]}-tagging", title_fontproperties={'weight':'bold'})
        ax[k, j].grid()
        ax[k, j].set_ylim(-3, 0)
        ax[k, j].set_xlabel("jet tagging efficiency")
        ax[k, j].set_ylabel("log10(jet misid. probability)")
    else:
        print("failed")

def log_multiline_rocs_s(y_true, y_score, labels, ax, k, j, ls="solid", l=True):
    q_tag = 3 # s-tagging
    vs_tag = [1,2, 4, 5] # ud, b, g
    # colors
    color_cycle = plt.rcParams['axes.prop_cycle'].by_key()['color']
    dic_c = {1: color_cycle[0], 2: color_cycle[1], 3: color_cycle[4], 4: color_cycle[3], 5: color_cycle[2]}
    _bg = create_binary_rocs(q_tag, vs_tag[0], y_true, y_score)
    _bud = create_binary_rocs(q_tag, vs_tag[1], y_true, y_score)
    _bc = create_binary_rocs(q_tag, vs_tag[2], y_true, y_score)
    _bs = create_binary_rocs(q_tag, vs_tag[3], y_true, y_score)

    if len(_bg) > 0 and len(_bud) > 0 and len(_bc) > 0 and len(_bs) > 0:
        xs = [_bg[1], _bud[1], _bc[1], _bs[1]]
        ys = [_bg[0], _bud[0], _bc[0], _bs[0]]
        auc_ = [_bg[2], _bud[2], _bc[2], _bs[2]]

        # plot
        ys_log = [np.log10(j + 1e-8) for j in ys]
        i = 0
        for x, y in zip(xs, ys_log):
            if l:
                label = f"{labels[q_tag]} vs {labels[vs_tag[i]]}"
            else:
                label=None
            ax[k, j].plot(x, y, label=label, color=dic_c[vs_tag[i]], linestyle=ls) #(AUC={auc_[i]:.3f})")
            i += 1
        ax[k, j].legend(title=f"{labels[q_tag]}-tagging", title_fontproperties={'weight':'bold'})
        ax[k, j].grid()
        ax[k, j].set_ylim(-3, 0)
        ax[k, j].set_xlabel("jet tagging efficiency")
        ax[k, j].set_ylabel("log10(jet misid. probability)")
    else:
        print("failed")

def log_multiline_rocs_g(y_true, y_score, labels, ax, k, j, ls="solid", l=True):
    q_tag = 5 # g-tagging
    vs_tag = [1,2, 3, 4] # ud, b, g
    # colors
    color_cycle = plt.rcParams['axes.prop_cycle'].by_key()['color']
    dic_c = {1: color_cycle[0], 2: color_cycle[1], 3: color_cycle[4], 4: color_cycle[3], 5: color_cycle[2]}
    _bg = create_binary_rocs(q_tag, vs_tag[0], y_true, y_score)
    _bud = create_binary_rocs(q_tag, vs_tag[1], y_true, y_score)
    _bc = create_binary_rocs(q_tag, vs_tag[2], y_true, y_score)
    _bs = create_binary_rocs(q_tag, vs_tag[3], y_true, y_score)

    if len(_bg) > 0 and len(_bud) > 0 and len(_bc) > 0 and len(_bs) > 0:
        xs = [_bg[1], _bud[1], _bc[1], _bs[1]]
        ys = [_bg[0], _bud[0], _bc[0], _bs[0]]
        auc_ = [_bg[2], _bud[2], _bc[2], _bs[2]]

        # plot
        ys_log = [np.log10(j + 1e-8) for j in ys]
        i = 0
        for x, y in zip(xs, ys_log):
            if l:
                label = f"{labels[q_tag]} vs {labels[vs_tag[i]]}"
            else:
                label=None
            ax[k, j].plot(x, y, label=label, color=dic_c[vs_tag[i]], linestyle=ls) #(AUC={auc_[i]:.3f})")
            i += 1
        ax[k, j].legend(title=f"{labels[q_tag]}-tagging", title_fontproperties={'weight':'bold'})
        ax[k, j].grid()
        ax[k, j].set_ylim(-3, 0)
        ax[k, j].set_xlabel("jet tagging efficiency")
        ax[k, j].set_ylabel("log10(jet misid. probability)")
    else:
        print("failed")

def get_y_true_and_score(data):
    labels = ['U', 'D', 'C', 'S', 'B', 'G', 'TAU']
    y_true = np.array([data[f'recojet_is{label}'] for label in labels])
    y_true = np.argmax(y_true.T, axis=1)
    y_score = np.array([data[f'score_recojet_is{label}'] for label in labels])
    #print(y_score.shape)
    ind_nan = invalid_ind(data)
    #print(ind_nan.shape)
    y_score = np.delete(y_score, ind_nan, axis=1)
    y_true = np.delete(y_true, ind_nan)
    return y_true, y_score.T

def all_rocs(data1, data2, label1, label2, save=False, name=None, bbox_anc=(0.6, 1.0)):
    labels = ['ud', 'ud', 'c', 's', 'b', 'g', 'tau']
    fig, ax = plt.subplots(2, 2, figsize=(12, 7), constrained_layout=True)
    y_true, y_score = get_y_true_and_score(data1)


    # Plot ROC curves for tagging
    log_multiline_rocs_b(y_true, y_score, labels, ax, 0, 0)
    log_multiline_rocs_c(y_true, y_score, labels, ax, 0, 1)
    log_multiline_rocs_s(y_true, y_score, labels, ax, 1, 0)
    log_multiline_rocs_g(y_true, y_score, labels, ax, 1, 1)

    y_true, y_score = get_y_true_and_score(data2)

    # Plot ROC curves for tagging
    log_multiline_rocs_b(y_true, y_score, labels, ax, 0, 0, ls="dashed", l=False)
    log_multiline_rocs_c(y_true, y_score, labels, ax, 0, 1, ls="dashed", l=False)
    log_multiline_rocs_s(y_true, y_score, labels, ax, 1, 0, ls="dashed", l=False)
    log_multiline_rocs_g(y_true, y_score, labels, ax, 1, 1, ls="dashed", l=False)


    # Create the first legend for colorful ROC curves
    handles_colorful, labels_colorful = ax[0,0].get_legend_handles_labels()
    legend_colorful = ax[0,0].legend(handles=handles_colorful, title=f"{labels[4]}-tagging", title_fontproperties={'weight':'bold'})

    # Create black lines for FullSim and FastSim
    fullsim_line = Line2D([0], [0], color='black', linestyle='-', label=label1)
    fastsim_line = Line2D([0], [0], color='black', linestyle='--', label=label2)

    # Add the second legend for FullSim and FastSim
    legend_sim = ax[0,0].legend(handles=[fullsim_line, fastsim_line], loc="upper center", bbox_to_anchor=bbox_anc)

    # Add back the first legend (colorful ROC curves) to the plot
    ax[0,0].add_artist(legend_colorful)
    # Re-enable the grid for all subplots
    for row in range(2):
        for col in range(2):
            ax[row, col].grid(True)  # Make sure the grid is visible

    if save:
        plt.savefig("{}/{}.pdf".format(path, name))
    else:
        plt.show()

# non-binary discriminates

def change_ud_to_l(y_score, y_true=None, keep_ud=False):
    # change u and d probabilites to (u+d)/2
    if keep_ud == False:
        mean_value = np.mean(y_score[:, :2], axis=1)
        print(mean_value.shape)
        y_score[:, 0] = mean_value
        y_score[:, 1] = mean_value
    elif keep_ud==True and y_true.any()!= None:
        i_not_u = np.where(y_true!=0)
        i_not_d = np.where(y_true!=1)
        i_not = np.intersect1d(i_not_u, i_not_d)

        mean_value = np.mean(y_score[i_not, :2], axis=1)
        y_score[i_not, 0] = mean_value
        y_score[i_not, 1] = mean_value
    else:
        raise ValueError("pass y_true if to keep ud")
    return y_score

def create_label_mapping():
    labels = ['ud', 'l', 'c', 's', 'b', 'g', 'tau'] # 0 and 1 are both light (l) but function needs to bijective

    # Map label to integer
    label_to_int = {label: idx for idx, label in enumerate(labels)}

    # Map integer to label
    int_to_label = {idx: label for idx, label in enumerate(labels)}

    return label_to_int, int_to_label

def non_binary_disc(data1, data2, dic, label1, label2, dicx=None, save=False, name=None, lax=0):
    fig, ax = plt.subplots(2, 2, figsize=(12, 7), constrained_layout=True)
    ax = ax.flat

    label_to_int, int_to_label = create_label_mapping()

    color_cycle = plt.rcParams['axes.prop_cycle'].by_key()['color']
    dic_c = {0: color_cycle[0], 2: color_cycle[1], 3: color_cycle[4], 4: color_cycle[3], 5: color_cycle[2]}
    ls = ["solid", "dashed"]

    for d, data in enumerate([data1, data2]):
        y_true, y_score = get_y_true_and_score(data)
        y_score = change_ud_to_l(y_score, y_true, True) # average ud to light

        for a, quark in enumerate(dic.keys()):
            # plot
            int_quark = label_to_int[quark]
            y_score_MC_b = y_score[np.where(y_true==int_quark)] # shape (x, 7)
            q_int = [label_to_int[q] for q in dic[quark]]

            for l, i in enumerate(q_int):
                score = y_score_MC_b[:, i]
                lab = None
                r = None
                hist_val = np.log10(score/(1 - score))
                if dicx:
                    r = dicx[quark]
                    if r:
                        hist_val = np.clip(hist_val, *r)
                if d==0:
                    lab = dic[quark][l]
                ax[a].hist(hist_val, histtype='step', linewidth=2.0, density=True, bins=60, label=lab, color=dic_c[i], linestyle=ls[d], range=r)
                ax[a].grid(True)
                ax[a].legend()
                ax[a].set_xlabel(r"${}$-jet discriminant".format(quark))

    # legends ...
    handles_colorful, labels_colorful = ax[lax].get_legend_handles_labels()
    legend_colorful = ax[lax].legend(handles=handles_colorful) #, title=f"{labels[4]}-tagging", title_fontproperties={'weight':'bold'})
    # Create black lines for FullSim and FastSim
    fullsim_line = Line2D([0], [0], color='black', linestyle='-', label=label1)
    fastsim_line = Line2D([0], [0], color='black', linestyle='--', label=label2)
    # Add the second legend for FullSim and FastSim
    legend_sim = ax[lax].legend(handles=[fullsim_line, fastsim_line], loc="upper left")#, bbox_to_anchor=(0.6, 1.0))
    # reset colorful legend
    ax[lax].add_artist(legend_colorful)
    if save:
        plt.savefig("{}/{}.pdf".format(path, name))
    else:
        plt.show()
