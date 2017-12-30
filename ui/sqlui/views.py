from django.shortcuts import render

# Create your views here.

def index(request):
    context = {}
    context['dblist'] = ['test1', 'test2']
    context['dir'] = '/'
    return render(request, "sqlui/database.html", context)

def database(request):
    context = {}
    context['tblist'] = ['test1', 'test2', 'test3']
    
    request.session['databaseName'] =  request.GET.get('databaseName')
    context['dir'] = '/' + request.GET.get('databaseName') + '/'
    context['databaseName'] = request.GET.get('databaseName')
    return render(request, "sqlui/table.html", context)

def table(request):
    context = {}

    tbinfo = {}
    tbinfo['thead'] = ['th1', 'th2', 'th3', 'th4']
    tbinfo['tbinfo'] = [{"tbitem":['t1', 't2', 't3', 't4']},{"tbitem":['test1', 'test2', 'test3', 'test4']}]
    
    context['tbgroup'] = [tbinfo,]
    request.session['tableName'] =  request.GET.get('tableName')
    context['dir'] = '/' + request.session.get('databaseName') + '/' + request.GET.get('tableName')
    return render(request, "sqlui/result.html", context)

def deldatabase(request):
    context = {}
    #todo delete database
    #todo show databaselist
    
    context['dblist'] = ['todo']  
    context['dir'] = '/'
    return render(request, "sqlui/database.html", context)

def adddatabase(request):
    context = {}
    #todo add database
    #todo show databaselist
    
    context['dblist'] = ['todo']  
    context['dir'] = '/'
    return render(request, "sqlui/database.html", context)

def deltable(request):
    context = {}
    #todo delete table
    #todo show tablelist
    
    context['tblist'] = ['todo']  
    
    context['databaseName'] = request.session.get('databaseName')
    context['dir'] = '/' + context['databaseName'] + '/'
    return render(request, "sqlui/table.html", context)

def addtable(request):
    context = {}
    #todo delete table
    #todo show tablelist
    
    context['tblist'] = ['todo']  
    context['databaseName'] = request.session.get('databaseName')
    context['dir'] = '/' + context['databaseName'] + '/'
    return render(request, "sqlui/table.html", context)

def addsql(request):
    context = {}
    #todo sql
    #todo show tbinfo
    
    tbinfo = {}
    tbinfo['thead'] = ['todo', 'todo', 'todo', 'todo', 'todo']
    tbinfo['tbinfo'] = [{"tbitem":['todo', 'todo', 'todo', 'todo', 'todo']},{"tbitem":['todo', 'todo', 'todo', 'todo', 'todo']}]
    
    context['tbgroup'] = [tbinfo,tbinfo]
    request.session['tableName'] =  request.session.get('tableName')
    context['dir'] = '/' + request.session.get('databaseName') + '/' + request.session.get('tableName')
    return render(request, "sqlui/result.html", context)