			if(isdigit(s[0]))
				{
				i=atoi(s);
				if(i <= numread && i > 0 && user.filesl >= fileidx[i-1].level)
					{
					if(fileidx[i-1].pass[0]!=0)
						{
						nl();
						put("1Enter file base password:2 ");
						input(k,50);
						if(stricmp(fileidx[i-1].pass,k)!=0)
							{
							nl();
							pl("4Sorry, wrong password");
							}
						else
							cur=i-1;
						}
					else
						cur=i-1;
					}
				}
			break;