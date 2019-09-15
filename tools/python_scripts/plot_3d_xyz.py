import pandas as pd
import plotly.express as px

path = "/home/jake/code/jake/TeensyFlight36/tools/raw_data/3_axis_data.csv";
df = pd.read_csv(path)
print(df)

# iris = px.data.iris()
# print(iris)
fig = px.scatter_3d(df, x='x', y='y', z='z')

fig.update_traces(marker=dict(size=2,
                              line=dict(width=2,
                                        color='DarkSlateGrey')),
                  selector=dict(mode='markers'))
# fig.show()
out = "/home/jake/code/jake/TeensyFlight36/tools/raw_data/mag_data.html";
fig.write_html(out, auto_open=True)
