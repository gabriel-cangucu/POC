import os
import json
from docplex.mp.model import Model
from time import time
from tqdm.auto import tqdm


def solve_kpf_model(params: dict) -> dict:
    kpf_model = Model(name='Knapsack Problem with Forfeits')
    kpf_model.set_time_limit(3600)

    # Defining decision variables
    x_vars = kpf_model.binary_var_list(params['number_of_items'], name='x')
    v_vars = kpf_model.binary_var_list(params['number_of_pairs'], name='v')

    # Defining constraints
    kpf_model.add_constraint(
        sum(params['weights'][i]*x_vars[i] for i in range(params['number_of_items'])) <= params['budget']
    )

    for k, (i, j) in enumerate(params['forfeit_pairs']):
        kpf_model.add_constraint(x_vars[i] + x_vars[j] - v_vars[k] <= 1)

    # Defining the objective function
    objective = sum(params['profits'][i]*x_vars[i] for i in range(params['number_of_items'])
            ) - sum(params['forfeits'][k]*v_vars[k] for k in range(params['number_of_pairs']))
        
    kpf_model.set_objective('max', objective)

    # Optimizing the model
    begin = time()
    solution = kpf_model.solve()
    end = time()

    # Calculating the number of forfeit pairs paid
    forfeits_paid = []

    for v in kpf_model.iter_binary_vars():
        if v.name.startswith('v'):
            forfeits_paid.append(v.solution_value)

    return {
        'solution': int(solution.get_objective_value()),
        'forfeits_paid': int(sum(forfeits_paid)),
        'time_elapsed': round(end - begin, 2)
    }


def store_results_to_csv(params: dict, results: dict, file_name: str) -> None:
    with open(file_name, 'a') as f:
        # If the file is empty, we add a header
        if os.stat(file_name).st_size == 0:
            f.write('n,id,sol,#forf.,time(s)\n')
        
        f.write(str(params['number_of_items']) + ',')
        f.write(str(params['instance_id']) + ',')
        f.write(str(results['solution']) + ',')
        f.write(str(results['forfeits_paid']) + ',')
        f.write(('TL' if results['time_elapsed'] >= 3600 else str(results['time_elapsed'])) + '\n')


if __name__ == '__main__':
    with open('instances.json', 'r') as f:
        json_data = json.load(f)

        for instance in tqdm(json_data):
            params = {
                'instance_id': instance['instance_id'],
                'number_of_items': instance['number_of_items'],
                'number_of_pairs': instance['number_of_pairs'],
                'budget': instance['budget'],
                'weights': instance['weights'],
                'profits': instance['profits'],
                'forfeits': instance['forfeits'],
                'forfeit_pairs': instance['forfeit_pairs']
            }

            results = solve_kpf_model(params)
            store_results_to_csv(params, results, file_name='cplex_results.csv')
