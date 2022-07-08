import random
import json


def generate_pairs(n: int, k: int) -> list:
    forfeit_pairs = []

    for _ in range(k):
        pair = set([random.randint(0, n-1), random.randint(0, n-1)])

        # If both items of the pair are equal or if they were already generated
        while len(pair) < 2 or pair in forfeit_pairs:
            pair = set([random.randint(0, n-1), random.randint(0, n-1)])
        
        forfeit_pairs.append(pair)

    return [list(pair) for pair in forfeit_pairs]


if __name__ == '__main__':
    instances = []

    for number_of_items in [500, 700, 800, 1000]:
        for i in range(10):
            number_of_pairs = 6*number_of_items
            budget = 3*number_of_items

            weights = [random.randint(3, 20) for _ in range(number_of_items)]
            profits = [random.randint(5, 25) for _ in range(number_of_items)]
            forfeits = [random.randint(2, 15) for _ in range(number_of_pairs)]
            forfeit_pairs = generate_pairs(n=number_of_items, k=number_of_pairs)

            instances.append({
                'instance_id': i+1,
                'number_of_items': number_of_items,
                'number_of_pairs': number_of_pairs,
                'budget': budget,
                'weights': weights,
                'profits': profits,
                'forfeits': forfeits,
                'forfeit_pairs': forfeit_pairs
            })

    with open('instances.json', 'w') as f:
        json.dump(instances, f, indent=4)
