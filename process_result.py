import os
import pickle
from argparse import ArgumentParser
import numpy as np
import scipy.sparse as ssp


def read_edgelist(path, nodes_dict, delimeter='\t', comment='#', hasValue=False, shape=None):
    rows, cols, datas = [], [], []
    k2idx = dict(zip(nodes_dict, range(len(nodes_dict))))
    for line in open(path, 'r'):
        line = line.strip()
        if len(line) == 0:
            continue
        if line.startswith(comment):
            continue
        elems = line.split(delimeter)
        if len(elems) < 2:
            print(f"Line does not match the format: {line}")
            return
        u, v = elems[:2]
        u, v = k2idx[int(u)], k2idx[int(v)]
        rows.append(u)
        cols.append(v)
        if hasValue and len(elems) > 2:
            datas.append(int(elems[2]))
        else:
            datas.append(1)
    sm = ssp.csr_matrix((datas, (rows, cols)), shape=shape)
    sm = sm.maximum(sm.T)
    return sm


def process_nodes_dict(path):
    nodes_dict = dict()
    for line in open(path, 'r'):
        line = line.strip()
        if len(line) == 0:
            continue
        elems = line.split('\t')
        elems = [int(x) for x in elems]
        nodes_dict[elems[0]] = set(elems)
    return nodes_dict


def construct_P(nodes_dict: dict):
    degs = []
    path = os.path.join('output', args.dataset, 'degrees.txt')
    for line in open(path, 'r'):
        line = line.strip()
        if len(line) == 0:
            continue
        degs.append(int(line))
    degs = np.array(degs)
    print(f"N: {len(degs)}")
    print(f"n: {len(nodes_dict)}")
    N = len(degs)
    np.save(os.path.join('output', args.dataset, 'degrees.npy'), degs)

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
    P = ssp.coo_matrix((datas, (rows, cols)), shape=(n, N))
    P_ = ssp.coo_matrix(([1] * len(rows), (cols, rows)), shape=(N, n))
    return P, P_


parser = ArgumentParser()
parser.add_argument('dataset', type=str, help='Dataset name')
parser.add_argument('--delimeter', type=str, default='\t',
                    help='Delimeter of edgelist file')
parser.add_argument('--comment', type=str, default='#',
                    help='Comment string of edgelist file')
args = parser.parse_args()

if __name__ == '__main__':
    path = os.path.join('./output', args.dataset, 'supernodes.txt')
    nodes_dict = process_nodes_dict(path)
    path = os.path.join('./output', args.dataset, 'nodes_dict.pkl')
    pickle.dump(nodes_dict, open(path, 'wb'))
    print("Save supernodes done!", len(nodes_dict))
    n = len(nodes_dict)

    path = os.path.join('./output', args.dataset, 'summary.edgelist')
    sm_s = read_edgelist(path, nodes_dict, args.delimeter,
                         args.comment, True, shape=(n, n))
    path = os.path.join('./output', args.dataset, 'sm_s.npz')
    ssp.save_npz(path, sm_s)
    print("Save sm done!", sm_s.shape)

    P, P_ = construct_P(nodes_dict)
    print(P.shape, P_.shape)
    path = os.path.join('./output', args.dataset, 'P.npz')
    ssp.save_npz(path, P)
    path = os.path.join('./output', args.dataset, 'P_.npz')
    ssp.save_npz(path, P_)
    print("Save matrix done!")
