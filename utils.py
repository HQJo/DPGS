import os
import pickle

import numpy as np
import scipy.sparse as ssp


def construct_P(degs: np.array, nodes_dict: dict, dataset: str, ratio: float):
    print(f"N: {len(degs)}")
    print(f"n: {len(nodes_dict)}")
    N = len(degs)
    # if ratio >= 0.1:
    #     dir_ = f"output/{dataset}_{ratio:.1f}"
    # else:
    #     dir_ = f"output/{dataset}_{ratio:.2f}"
    dir_ = f"output/{dataset}_{ratio:.2f}"
    np.save(os.path.join(dir_, 'degrees.npy'), degs)
    # degs += 1

    rows, cols, datas = [], [], []
    for i, nodes in enumerate(nodes_dict.values()):
        assert len(nodes) != 0
        D = sum(degs[n_] for n_ in nodes)
        nodes = list(nodes)
        rows.extend([i] * len(nodes))
        cols.extend(nodes)
        if D == 0:
            datas.extend([1.0 / len(nodes)] * len(nodes))
        else:
            datas.extend((degs[n_] / D for n_ in nodes))
    n = len(nodes_dict)
    P = ssp.coo_matrix(([1] * len(rows), (rows, cols)), shape=(n, N))
    # P_ = ssp.coo_matrix((datas, (cols, rows)), shape=(n, N))
    ssp.save_npz(os.path.join(dir_, 'P.npz'), P)
    # ssp.save_npz(os.path.join(dir_, 'P_.npz'), P_)
    return P


def post_process(adj: ssp.spmatrix, nodes_dict: dict, dataset: str, ratio: float):
    # if ratio >= 0.1:
    #     dir_ = f"output/{dataset}_{ratio:.1f}"
    # else:
    #     dir_ = f"output/{dataset}_{ratio:.2f}"
    dir_ = f"output/{dataset}_{ratio:.2f}"
    if not os.path.exists(dir_):
        os.mkdir(dir_)
    path = os.path.join(dir_, 'nodes_dict.pkl')
    pickle.dump(nodes_dict, open(path, 'wb'))

    adj = adj + ssp.eye(adj.shape[0])
    degs = np.array(adj.sum(axis=1)).squeeze()

    P = construct_P(degs, nodes_dict, dataset, ratio)
    adj_s = P @ (adj @ P.T)
    ssp.save_npz(os.path.join(dir_, 'adj_s.npz'), adj_s)
